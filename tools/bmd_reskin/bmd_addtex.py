"""
bmd_addtex -- inject textures from a donor BMD into a target BMD's TEX1 and repoint a
material's texture slots. Built to restore Armogohma's eye environment/emboss maps
(goma_kankyo, eye_enbos) that the Blender->SuperBMD pipeline can't embed (it only keeps
mesh-diffuse textures), which left the eye's TEV stage 1 clamping to pure white.

Strategy (TEX1 + MAT3 are the last two sections, so we can rebuild the tail):
  * TEX1: rebuild = existing textures + donor textures (copy BTI header + image data,
    fix each header's dataOffset).
  * MAT3: append an extended texNo block (old entries + new TEX1 indices) at the section
    end, repoint the texNo header offset to it, and patch the target material's
    texNo-index slots. Nothing else in MAT3 moves.
Verifies offline: re-parses the result, checks texture names/count and the material's
resolved texture indices, and spot-checks image-data bytes vs the donor.
"""
import struct, os
import numpy as np  # noqa (kept for parity with bmd_reskin env)

def u16(b,o): return struct.unpack_from('>H',b,o)[0]
def u32(b,o): return struct.unpack_from('>I',b,o)[0]
def s16(b,o): return struct.unpack_from('>h',b,o)[0]

FMT_BPP={0:0.5,1:1,2:1,3:2,4:2,5:2,6:4,8:0.5,9:1,10:2,14:0.5}  # GX tex format -> bytes/texel
FMT_NAME={0:'I4',1:'I8',2:'IA4',3:'IA8',4:'RGB565',5:'RGB5A3',6:'RGBA32',8:'C4',9:'C8',10:'C14X2',14:'CMPR'}

def j3d_hash(s):
    h=0
    for c in s.encode('ascii'): h=(h*3+c)&0xFFFF
    return h

class Tex1:
    """Parsed TEX1: list of (name, header_bytes[0x20], image_bytes)."""
    def __init__(self, b, base):
        self.base=base; n=u16(b,base+8)
        hdrOff=base+u32(b,base+0x0C); strOff=base+u32(b,base+0x10)
        sc=u16(b,strOff); names=[]
        for i in range(sc):
            so=u16(b,strOff+4+i*4+2); e=b.index(0,strOff+so); names.append(b[strOff+so:e].decode('ascii'))
        self.textures=[]
        for i in range(n):
            h=hdrOff+i*0x20; hdr=bytearray(b[h:h+0x20])
            fmt=hdr[0]; w=u16(hdr,2); ht=u16(hdr,4); dataOff=u32(hdr,0x1C)
            palNum=u16(hdr,0x0A)
            assert palNum==0, f"texture {names[i]} is paletted (palNum={palNum}) -- unsupported"
            size=int(w*ht*FMT_BPP[fmt])
            img=bytes(b[h+dataOff:h+dataOff+size])
            self.textures.append([names[i],hdr,img,fmt,w,ht])

def build_tex1(textures):
    """textures: list of [name, hdr(0x20 bytearray), img, fmt, w, h]. Returns TEX1 section bytes."""
    n=len(textures)
    hdrOff=0x20
    dataStart=hdrOff+n*0x20
    # lay out image data (dedup identical blobs)
    data=bytearray(); dpos={}; offsets=[]
    for t in textures:
        img=t[2]; key=bytes(img)
        if key in dpos: offsets.append(dpos[key])
        else:
            off=len(data); dpos[key]=off; data+=img; offsets.append(off)
    dataAbsStart=dataStart
    # string table after data
    strStart=dataStart+len(data)
    # (align data to 0x20)
    while (dataStart+len(data))%0x20: data+=b'\x00'
    strStart=dataStart+len(data)
    # build headers with fixed dataOffset
    hdrs=bytearray()
    for i,t in enumerate(textures):
        hdr=bytearray(t[1])
        absData=dataStart+offsets[i]
        hpos=hdrOff+i*0x20
        struct.pack_into('>I',hdr,0x1C, absData-hpos)   # dataOffset relative to this header
        hdrs+=hdr
    # string table
    st=bytearray(); st+=struct.pack('>HH',n,0xFFFF)
    names=[t[0] for t in textures]
    # offsets relative to string-table start; strings after the (count+pad+entries) block
    body_off=4+n*4
    strbytes=bytearray(); stroffs=[]
    for nm in names:
        stroffs.append(body_off+len(strbytes)); strbytes+=nm.encode('ascii')+b'\x00'
    for i,nm in enumerate(names):
        st+=struct.pack('>HH', j3d_hash(nm), stroffs[i])
    st+=strbytes
    while len(st)%4: st+=b'\x00'
    # assemble
    body=bytearray(0x20)  # section header placeholder
    body+=hdrs
    body+=data
    assert len(body)==strStart
    body+=st
    while len(body)%0x20: body+=b'\x00'
    struct.pack_into('>4sI',body,0,b'TEX1',len(body))
    struct.pack_into('>H',body,8,n)
    struct.pack_into('>I',body,0x0C,hdrOff)
    struct.pack_into('>I',body,0x10,strStart)
    return bytes(body)

def addtex(target_path, donor_path, out_path, donor_names, mat_name, slot_map):
    """donor_names: texture names to copy from donor. mat_name: material to patch.
       slot_map: {texmap_slot: donor_name} to point that slot at the injected texture."""
    tb=bytearray(open(target_path,'rb').read())
    db=open(donor_path,'rb').read()
    # section table (sequential)
    def sections(buf):
        n=u32(buf,0x0C); sec={}; o=0x20; order=[]
        for _ in range(n):
            mg=buf[o:o+4].decode('ascii','replace'); sz=u32(buf,o+4); sec[mg]=(o,sz); order.append(mg); o+=sz
        return sec,order
    tsec,torder=sections(tb); dsec,_=sections(db)
    T=Tex1(tb,tsec['TEX1'][0]); D=Tex1(db,dsec['TEX1'][0])
    dmap={t[0]:t for t in D.textures}
    base_n=len(T.textures)
    newtex=[t[:] for t in T.textures]
    new_index={}
    for nm in donor_names:
        assert nm in dmap, f"{nm} not in donor"
        new_index[nm]=len(newtex); newtex.append([x for x in dmap[nm]])
    new_tex1=build_tex1(newtex)

    # --- MAT3 edit ---
    m0,msz=tsec['MAT3']; M=bytearray(tb[m0:m0+msz])
    offs=[u32(M,0x0C+i*4) for i in range(30)]
    TEXNO=15
    # existing texNo block: read entries until we hit the next block (offs[16])
    tn0=offs[TEXNO]; nextoff=min(o for o in offs[16:] if o>tn0)
    old_entries=[u16(M,tn0+j*2) for j in range((nextoff-tn0)//2)]
    # keep only the meaningful leading entries (0..base_n-1 map 1:1 typically = [0,1,2,...])
    keep=old_entries[:base_n]
    new_block=keep+[new_index[nm] for nm in donor_names]  # e.g. [0,1,2,3,4]
    # append block at end of MAT3 (aligned)
    while len(M)%4: M+=b'\x00'
    new_tn_off=len(M)
    for v in new_block: M+=struct.pack('>H',v)
    while len(M)%0x20: M+=b'\x00'
    struct.pack_into('>I',M,0x0C+TEXNO*4,new_tn_off)   # repoint texNo header offset
    struct.pack_into('>I',M,4,len(M))                  # new MAT3 size
    # patch material's texNo-index slots
    matData=offs[0]; nameTab=offs[2]
    nc=u16(M,nameTab); mnames=[]
    for i in range(nc):
        so=u16(M,nameTab+4+i*4+2); e=M.index(0,nameTab+so); mnames.append(M[nameTab+so:e].decode('ascii'))
    mi=mnames.index(mat_name);
    remap=u16(M,offs[1]+mi*2)
    st=matData+remap*0x14c
    for slot,nm in slot_map.items():
        block_idx=new_block.index(new_index[nm])
        struct.pack_into('>H',M,st+0x84+slot*2,block_idx)

    # --- reassemble: sections before MAT3 unchanged, then new MAT3, then new TEX1 ---
    out=bytearray(tb[:m0])       # header + INF1..SHP1
    out+=M                        # new MAT3
    out+=new_tex1                 # new TEX1
    struct.pack_into('>I',out,8,len(out))  # file size
    open(out_path,'wb').write(out)
    return dict(base_n=base_n,new_index=new_index,new_block=new_block,mat=mat_name,mi=mi)

def verify(out_path, donor_path, donor_names, mat_name, slot_map):
    ob=open(out_path,'rb').read(); db=open(donor_path,'rb').read()
    def sections(buf):
        n=u32(buf,0x0C); sec={}; o=0x20
        for _ in range(n):
            mg=buf[o:o+4].decode('ascii','replace'); sz=u32(buf,o+4); sec[mg]=(o,sz); o+=sz
        return sec
    osec=sections(ob); dsec=sections(db)
    OT=Tex1(ob,osec['TEX1'][0]); DT=Tex1(db,dsec['TEX1'][0]); dmap={t[0]:t for t in DT.textures}
    names=[t[0] for t in OT.textures]
    # image-data spot check for injected textures
    imgok=all(OT.textures[[t[0] for t in OT.textures].index(nm)][2]==dmap[nm][2] for nm in donor_names)
    # material resolved indices
    m0=osec['MAT3'][0]; offs=[u32(ob,m0+0x0C+i*4) for i in range(30)]
    nameTab=m0+offs[2]; nc=u16(ob,nameTab); mnames=[]
    for i in range(nc):
        so=u16(ob,nameTab+4+i*4+2); e=ob.index(0,nameTab+so); mnames.append(ob[nameTab+so:e].decode('ascii'))
    mi=mnames.index(mat_name); remap=u16(ob,m0+offs[1]+mi*2); st=m0+offs[0]+remap*0x14c
    tn=m0+offs[15]
    slots=[u16(ob,st+0x84+k*2) for k in range(8)]
    resolved=[ (u16(ob,tn+s*2) if s!=0xFFFF else None) for s in slots]
    return names, imgok, slots, resolved


if __name__=='__main__':
    import argparse
    here=os.path.dirname(__file__)
    ap=argparse.ArgumentParser(description="Inject env/emboss textures into a BMD and repoint a material's texture slots.")
    ap.add_argument('--target', default=os.path.join(here,'B_gm_37_reskinned.bmd'), help='BMD to add textures to (e.g. the reskinned reveal).')
    ap.add_argument('--donor',  default=r'D:\XXXXXXX\Ex TP\Blender workflow\B-gmarc\bmdv\goma.bmd', help='BMD to copy textures from (vanilla).')
    ap.add_argument('--out',    default=os.path.join(here,'B_gm_37_final.bmd'))
    a=ap.parse_args()
    donor_names=['goma_kankyo','eye_enbos']
    mat='goma2_eye_mat01'
    slot_map={1:'goma_kankyo', 2:'eye_enbos'}   # texmap1=env, texmap2=emboss (texmap0 stays eye01)
    r=addtex(a.target,a.donor,a.out,donor_names,mat,slot_map)
    print(f"addtex -> {a.out}")
    print(f"  injected {donor_names} as TEX1 indices {r['new_index']}; new texNo block {r['new_block']}")
    names,imgok,slots,resolved=verify(a.out,a.donor,donor_names,mat,slot_map)
    print(f"  TEX1 now: {names}")
    print(f"  eye '{mat}' texNo-index slots: {[hex(s) for s in slots]}")
    print(f"  -> resolved TEX1 indices: {[resolved[i] for i in range(3)]}  (expect eye01=1, kankyo, enbos)")
    print(f"  injected image-data bytes match donor: {imgok}")
    ok = imgok and resolved[0]==1 and resolved[1]==r['new_index']['goma_kankyo'] and resolved[2]==r['new_index']['eye_enbos']
    print("  ==> PASS" if ok else "  ==> *** FAIL ***")
