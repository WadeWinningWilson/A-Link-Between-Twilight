"""
BMD reskin -- transplant vanilla joint frames into a Blender-round-tripped BMD and
re-express vertices so the original game's shared animations drive it correctly again.

Root cause it fixes: importing a rigged BMD into Blender re-derives every bone's local
frame (Blender bones point +Y). Bone *positions* survive, frames don't. Rigid verts are
stored joint-local (J3DMtxBuffer::calcDrawMtx rigid path = viewMtx * jointWorld, no
inverse-bind), so they're welded to the wrong frames -> vanilla anims tear the mesh.

Fix: for every non-eye/lid joint, set the frame to vanilla's (keeping the reveal model's
+90X root space), re-express each rigid vertex/normal by S_J = targetWorld_J^-1 * revealWorld_J
(a pure rotation, positions coincide), and rebuild EVP1 inverse-binds. Eye/lids/root keep
their world (eye is code-anchored). Verified offline by model-space rest-position invariance.
"""
import struct, os
import numpy as np

def rd_u32(b,o): return struct.unpack_from('>I',b,o)[0]
def rd_u16(b,o): return struct.unpack_from('>H',b,o)[0]
def rd_s16(b,o): return struct.unpack_from('>h',b,o)[0]
def rd_u8(b,o):  return b[o]
def rd_f32(b,o): return struct.unpack_from('>f',b,o)[0]
def find_magic(b, magic): return b.find(magic.encode('ascii'))
def s2r(s): return s/32768.0*np.pi
def rotX(a):
    c,s=np.cos(a),np.sin(a); return np.array([[1,0,0,0],[0,c,-s,0],[0,s,c,0],[0,0,0,1]],float)
def rotY(a):
    c,s=np.cos(a),np.sin(a); return np.array([[c,0,s,0],[0,1,0,0],[-s,0,c,0],[0,0,0,1]],float)
def rotZ(a):
    c,s=np.cos(a),np.sin(a); return np.array([[c,-s,0,0],[s,c,0,0],[0,0,1,0],[0,0,0,1]],float)
def trans(x,y,z):
    m=np.eye(4); m[0,3]=x; m[1,3]=y; m[2,3]=z; return m

def decompose_zyx(m):
    """rotation part -> ZYX-euler s16 (matches R=Rz@Ry@Rx) + translation."""
    R=m[:3,:3]
    sy=max(-1.0,min(1.0,-R[2,0])); y=np.arcsin(sy)
    if abs(R[2,0])<0.99999:
        x=np.arctan2(R[2,1],R[2,2]); z=np.arctan2(R[1,0],R[0,0])
    else:
        x=np.arctan2(-R[1,2],R[1,1]); z=0.0
    def r2s(a): return max(-32768,min(32767,int(round(a/np.pi*32768.0))))
    return (m[0,3],m[1,3],m[2,3], r2s(x),r2s(y),r2s(z))


class BMD:
    def __init__(self, path):
        self.path=path; self.b=bytearray(open(path,'rb').read())
        self._sections()
        self._inf1(); self._jnt1(); self._drw1(); self._evp1(); self._vtx1(); self._shp1()

    def _sections(self):
        """Walk the BMD header's section chain (robust vs content-search false matches)."""
        b=self.b; n=rd_u32(b,0x0C); self.sec={}; o=0x20
        for _ in range(n):
            mg=b[o:o+4].decode('ascii','replace'); sz=rd_u32(b,o+4)
            self.sec[mg]=o; o+=sz
    def _m(self,magic): return self.sec[magic]

    def _inf1(self):
        b=self.b; inf=self._m('INF1'); p=inf+rd_u32(b,inf+0x14)
        stack=[]; cur=-1; last=-1; self.parent={}; self.joint_order=[]
        while True:
            t=rd_u16(b,p); v=rd_u16(b,p+2); p+=4
            if t==0: break
            elif t==1: stack.append(cur); cur=last
            elif t==2: cur=stack.pop()
            elif t==0x10: self.parent[v]=cur; last=v; self.joint_order.append(v)

    def _jnt1(self):
        b=self.b; jnt=self._m('JNT1'); self.njoints=rd_u16(b,jnt+8)
        base=jnt+rd_u32(b,jnt+0x0C); self.joints=[]
        for i in range(self.njoints):
            e=base+i*0x40
            self.joints.append(dict(off=e,
                sx=rd_f32(b,e+0x04),sy=rd_f32(b,e+0x08),sz=rd_f32(b,e+0x0C),
                rx=rd_s16(b,e+0x10),ry=rd_s16(b,e+0x12),rz=rd_s16(b,e+0x14),
                tx=rd_f32(b,e+0x18),ty=rd_f32(b,e+0x1C),tz=rd_f32(b,e+0x20)))

    def local_mtx(self,j):
        return trans(j['tx'],j['ty'],j['tz'])@rotZ(s2r(j['rz']))@rotY(s2r(j['ry']))@rotX(s2r(j['rx']))@np.diag([j['sx'],j['sy'],j['sz'],1.0])

    def fk(self,joints=None):
        joints=joints if joints is not None else self.joints; w={}
        for i in self.joint_order:
            L=self.local_mtx(joints[i]); p=self.parent[i]; w[i]=L if p<0 else w[p]@L
        return w

    def _drw1(self):
        b=self.b; d=self._m('DRW1'); self.ndraw=rd_u16(b,d+8)
        offW=rd_u32(b,d+0x0C); offI=rd_u32(b,d+0x10)
        self.drw_isweighted=[rd_u8(b,d+offW+i) for i in range(self.ndraw)]
        self.drw_index=[rd_u16(b,d+offI+i*2) for i in range(self.ndraw)]

    def _evp1(self):
        b=self.b; e=self._m('EVP1'); self.nevp=rd_u16(b,e+8)
        self.evp_counts=[rd_u8(b,e+rd_u32(b,e+0x0C)+i) for i in range(self.nevp)]
        self.evp_mat_off=e+rd_u32(b,e+0x18)

    def inv_bind(self,j):
        o=self.evp_mat_off+j*48; m=np.eye(4)
        for r in range(3):
            for c in range(4): m[r,c]=rd_f32(self.b,o+(r*4+c)*4)
        return m
    def write_inv_bind(self,j,m):
        o=self.evp_mat_off+j*48
        for r in range(3):
            for c in range(4): struct.pack_into('>f',self.b,o+(r*4+c)*4,float(m[r,c]))

    def _vtx1(self):
        b=self.b; v=self._m('VTX1'); self.vtx1=v
        arrFmtOff=rd_u32(b,v+0x08); offs=[rd_u32(b,v+0x0C+i*4) for i in range(13)]
        fmts={}; p=v+arrFmtOff
        while True:
            a=rd_u32(b,p); comp=rd_u32(b,p+4); ct=rd_u32(b,p+8); p+=16
            if a==0xFF: break
            fmts[a]=dict(comp=comp,ctype=ct)
        self.vtx_fmts=fmts
        slotmap={9:0,10:1,11:3,12:4,13:5,14:6,15:7,16:8,17:9,18:10,19:11,20:12}
        self.vtx_attr_offset={a:v+offs[slotmap[a]] for a in fmts if a in slotmap}
        self._voffs=sorted(set(self.vtx_attr_offset.values()))
    def _stride(self,a):
        if a==9: return (3 if self.vtx_fmts[9]['comp']==1 else 2)*4   # F32 XYZ pos
        if a==10: return 3*2                                          # S16 XYZ normal
        return 12
    def npos(self):
        i=self._voffs.index(self.vtx_attr_offset[9])
        end=self._voffs[i+1] if i+1<len(self._voffs) else self.vtx1+rd_u32(self.b,self.vtx1+4)
        return (end-self.vtx_attr_offset[9])//self._stride(9)
    def read_pos(self,idx):
        o=self.vtx_attr_offset[9]+idx*self._stride(9)
        return np.array([rd_f32(self.b,o),rd_f32(self.b,o+4),rd_f32(self.b,o+8),1.0])
    def write_pos(self,idx,v):
        o=self.vtx_attr_offset[9]+idx*self._stride(9)
        for k in range(3): struct.pack_into('>f',self.b,o+k*4,float(v[k]))
    _NRM_SCALE=16384.0  # S16 frac=14
    def read_nrm(self,idx):
        o=self.vtx_attr_offset[10]+idx*6
        return np.array([rd_s16(self.b,o+k*2)/self._NRM_SCALE for k in range(3)])
    def write_nrm(self,idx,v):
        o=self.vtx_attr_offset[10]+idx*6
        for k in range(3):
            raw=int(round(float(v[k])*self._NRM_SCALE)); raw=max(-32768,min(32767,raw))
            struct.pack_into('>h',self.b,o+k*2,raw)

    def _shp1(self):
        b=self.b; s=self._m('SHP1'); self.nshapes=rd_u16(b,s+8)
        self.shp_off=s+rd_u32(b,s+0x0C); self.shp_attr_off=s+rd_u32(b,s+0x18)
        self.shp_mtxidx_off=s+rd_u32(b,s+0x1C); self.shp_prim_off=s+rd_u32(b,s+0x20)
        self.shp_mtxdata_off=s+rd_u32(b,s+0x24); self.shp_pktloc_off=s+rd_u32(b,s+0x28)
    def _desc(self,attr_offset):
        b=self.b; p=self.shp_attr_off+attr_offset; out=[]
        while True:
            a=rd_u32(b,p); dt=rd_u32(b,p+4); p+=8
            if a==0xFF: break
            out.append((a,dt))
        return out
    def build_vert_joint_map(self):
        """Returns pos2joint, nrm2joint, pos_weighted, nrm_weighted, conflicts(pos), nconflicts(nrm)."""
        b=self.b; pos2joint={}; nrm2joint={}; pos_w=set(); nrm_w=set(); conf=set(); nconf=set()
        for si in range(self.nshapes):
            se=self.shp_off+si*0x28
            packetCount=rd_u16(b,se+0x02); attrOffset=rd_u16(b,se+0x04)
            firstMatrix=rd_u16(b,se+0x06); firstPacket=rd_u16(b,se+0x08)
            desc=self._desc(attrOffset)
            for pk in range(packetCount):
                md=self.shp_mtxdata_off+(firstMatrix+pk)*8
                mcount=rd_u16(b,md+2); mfirst=rd_u32(b,md+4)
                mtxtable=[rd_u16(b,self.shp_mtxidx_off+(mfirst+k)*2) for k in range(mcount)]
                pl=self.shp_pktloc_off+(firstPacket+pk)*8
                psize=rd_u32(b,pl+0); poff=rd_u32(b,pl+4)
                p=self.shp_prim_off+poff; end=p+psize
                while p<end:
                    op=rd_u8(b,p); p+=1
                    if op==0: continue
                    vcount=rd_u16(b,p); p+=2
                    for _ in range(vcount):
                        posidx=nrmidx=None; mtxslot=0
                        for (a,dt) in desc:
                            sz=2 if dt==3 else 1
                            val=rd_u8(b,p) if sz==1 else rd_u16(b,p)
                            if a==0: mtxslot=val//3
                            elif a==9: posidx=val
                            elif a==10: nrmidx=val
                            p+=sz
                        drw=mtxtable[mtxslot]
                        if self.drw_isweighted[drw]:
                            pos_w.add(posidx); nrm_w.add(nrmidx)
                        else:
                            j=self.drw_index[drw]
                            if posidx in pos2joint and pos2joint[posidx]!=j: conf.add(posidx)
                            pos2joint[posidx]=j
                            if nrmidx is not None:
                                if nrmidx in nrm2joint and nrm2joint[nrmidx]!=j: nconf.add(nrmidx)
                                nrm2joint[nrmidx]=j
        return pos2joint,nrm2joint,pos_w,nrm_w,conf,nconf

    def write_joint_trs(self,i,tx,ty,tz,rx,ry,rz):
        e=self.joints[i]['off']
        struct.pack_into('>h',self.b,e+0x10,int(rx)); struct.pack_into('>h',self.b,e+0x12,int(ry)); struct.pack_into('>h',self.b,e+0x14,int(rz))
        struct.pack_into('>f',self.b,e+0x18,float(tx)); struct.pack_into('>f',self.b,e+0x1C,float(ty)); struct.pack_into('>f',self.b,e+0x20,float(tz))
    def copy_joint_trs_from(self,i,other):
        j=other.joints[i]; self.write_joint_trs(i,j['tx'],j['ty'],j['tz'],j['rx'],j['ry'],j['rz'])
    def save(self,path): open(path,'wb').write(self.b)


def reskin(source_path, vanilla_path, out_path, keep=(), evp_joints=()):
    """source_path: Blender-round-tripped BMD (custom mesh, re-derived frames).
    vanilla_path : the ORIGINAL game BMD whose animations must drive it.
    keep         : joints whose WORLD to preserve instead of adopting vanilla's frame
                   (moved/added joints, e.g. a relocated eye). Root is always kept.
    evp_joints   : joint indices referenced by EVP1 weighted envelopes (rebuild their
                   inverse-bind). Read them from the tool's parse output (evp indices)."""
    R=BMD(source_path); V=BMD(vanilla_path)
    wr=R.fk(); wv=V.fk()
    root=[i for i in R.joint_order if R.parent[i]<0][0]
    KEPT={root}|set(keep)
    Rroot=wr[root]                       # source root frame (e.g. +90X from --rotate); preserved
    tw={i:(wr[i].copy() if i in KEPT else Rroot@wv[i]) for i in R.joint_order}
    tl={i:(tw[i] if R.parent[i]<0 else np.linalg.inv(tw[R.parent[i]])@tw[i]) for i in R.joint_order}
    S={i:np.linalg.inv(tw[i])@wr[i] for i in R.joint_order}
    p2j,n2j,pw,nw,conf,nconf=R.build_vert_joint_map()
    assert not conf, f"position-joint conflicts: {conf}"
    for i in R.joint_order:
        if i==root: continue
        if i in KEPT: R.write_joint_trs(i,*decompose_zyx(tl[i]))   # reparented under vanilla frames
        else:         R.copy_joint_trs_from(i,V)                    # adopt vanilla local exactly
    changed=0
    for idx,j in p2j.items():
        if np.allclose(S[j],np.eye(4),atol=1e-6): continue
        R.write_pos(idx,S[j]@R.read_pos(idx)); changed+=1
    nch=0
    for idx,j in n2j.items():
        if np.allclose(S[j][:3,:3],np.eye(3),atol=1e-6): continue
        R.write_nrm(idx,S[j][:3,:3]@R.read_nrm(idx)); nch+=1
    # Weighted (EVP1) inverse-binds: carry the ORIGINAL bind to the new frame with the same S_J
    # used for rigid verts (do NOT assume invBind == inv(world_rest) -- it isn't; abdomen weights
    # use a different bind convention). Read old before writing (EVP1 not yet modified here).
    for j in evp_joints: R.write_inv_bind(j, S[j] @ R.inv_bind(j))
    R.save(out_path)
    return dict(R=R,V=V,tw=tw,tl=tl,S=S,p2j=p2j,changed=changed,nch=nch,nconf=nconf,root=root,KEPT=KEPT)


def verify(out_path, source_path, vanilla_path, keep=(), evp_joints=()):
    A=BMD(source_path); wr=A.fk(); p2j=A.build_vert_joint_map()[0]
    orig={idx:(wr[j]@A.read_pos(idx))[:3] for idx,j in p2j.items()}
    B=BMD(out_path); wt=B.fk(); maxd=0.0; worst=None
    for idx,j in p2j.items():                       # rigid: model-space rest preserved
        d=np.linalg.norm((wt[j]@B.read_pos(idx))[:3]-orig[idx])
        if d>maxd: maxd=d; worst=(idx,j)
    # weighted (EVP1): per-joint rest condition  wt[j]@newInvBind == wr[j]@oldInvBind
    ew=0.0
    for j in evp_joints:
        ew=max(ew,np.abs((wt[j]@B.inv_bind(j))-(wr[j]@A.inv_bind(j))).max())
    V=BMD(vanilla_path); wv=V.fk()
    root=[i for i in B.joint_order if B.parent[i]<0][0]; Rroot=wr[root]; KEPT={root}|set(keep); fr=0.0
    for i in B.joint_order:
        if i in KEPT: continue
        fr=max(fr,np.linalg.norm((Rroot@wv[i])-wt[i]))
    return maxd,worst,fr,ew


def _hexset(s):
    return tuple(int(x,0) for x in s.split(',')) if s else ()

if __name__=='__main__':
    import argparse
    here=os.path.dirname(__file__)
    ap=argparse.ArgumentParser(description="Transplant a vanilla BMD's bone frames into a Blender-round-tripped BMD so shared animations work; re-express verts and verify offline.")
    ap.add_argument('--source', default=os.path.join(here,'B_gm_37_prereskin_SOURCE.bmd'),
                    help='Blender-edited BMD (custom mesh). NOT an already-reskinned file (would double-reskin).')
    ap.add_argument('--vanilla', default=r'D:\XXXXXXX\Ex TP\Blender workflow\B-gmarc\bmdv\goma.bmd',
                    help='Original game BMD whose animations must drive the model.')
    ap.add_argument('--out', default=os.path.join(here,'B_gm_37_reskinned.bmd'))
    ap.add_argument('--keep', default='0x15,0x16,0x17',
                    help='Comma joint indices whose WORLD to preserve (moved/added joints). Root always kept. Default = Gohma eye/eyelids.')
    ap.add_argument('--evp', default='1,2,3',
                    help='Comma joint indices referenced by EVP1 weighted envelopes (rebuild their inverse-bind). Default = Gohma.')
    a=ap.parse_args()
    keep=_hexset(a.keep); evp=_hexset(a.evp)
    r=reskin(a.source,a.vanilla,a.out,keep=keep,evp_joints=evp)
    print(f"reskin -> {a.out}")
    print(f"  root (kept +root frame): 0x{r['root']:02X}   kept-world joints: {[hex(i) for i in sorted(r['KEPT'])]}")
    print(f"  rigid positions re-expressed: {r['changed']}/{len(r['p2j'])}  normals: {r['nch']}  nrm-conflicts: {len(r['nconf'])}")
    maxd,worst,fr,ew=verify(a.out,a.source,a.vanilla,keep=keep,evp_joints=evp)
    print(f"  VERIFY rigid rest-position drift: max {maxd:.4f}u (worst joint 0x{worst[1]:02X})")
    print(f"  VERIFY weighted (EVP1) rest residual: max {ew:.5f}")
    print(f"  VERIFY frame-match to vanilla: max {fr:.4f}")
    print("  ==> PASS" if (maxd<0.5 and fr<0.5 and ew<0.01) else "  ==> *** FAIL ***")
