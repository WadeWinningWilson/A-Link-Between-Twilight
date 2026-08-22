# d_a_obj_aygr decode campaign — FULLY MATCHED

Opened and closed 2026-08-22 (SEVENTH TU this session, THIRD consecutive
100). **18/18 exact, unit fuzzy 100.00** (WWDP a5977e1f).

Identity: the lookout platform / watchtower object ("Aygr" arc; the
assert strings name the two models yagura 櫓 and hashigo 梯子 — tower
and ladder). Param bit 0 gates the ladder model + its own dBgW
collision on top of the MoveBgActor base collision (res: 4 = tower
bdl, 5 = ladder bdl, 8 = base dzb, 9 = ladder dzb; MoveBGCreate heap
0x85F0).

## Method

Whole TU written in one pass against the matched family templates
(d_a_obj_ebomzo for the MoveBgActor virtual set + Mthd_* thunk pattern,
d_a_obj_akabe for the bgw Set/mtx style, d_a_obj_Yboil for the
daObj::PrmAbstract enum spelling). First compile 13/18 @ 99.30, exact
after two fixes.

## Levers

1. **u8 accessor return**: an inline prm accessor that the caller tests
   with `clrlwi.` (byte-mask record form) returns u8 in the donor —
   int returns give `cmpwi r3,0`, bool returns ALSO give cmpwi (MWCC
   folds the bool conversion of a known-1-bit AND result); only u8
   emits the truncate-and-test. Sibling of the s8/u8/bool type-tell
   family.
2. **Signed-addi size tell**: `lis r7,1; addi r7,r7,-31248` is 0x85F0,
   NOT 0x185F0 — addi sign-extends; read lis/addi pairs as
   (hi << 16) + (s16)lo, always.
3. dBgW `|= 0x80` at +0x6C = cBgW::SetLock().
4. Family templates make MoveBgActor TUs nearly free: virtuals
   CreateHeap/Create/Execute(Mtx**)/Draw/Delete at vtbl +8..+0x18,
   Mthd_Execute thunk calls MoveBGExecute directly, Mthd_Draw/IsDelete
   thunks dispatch virtually.
