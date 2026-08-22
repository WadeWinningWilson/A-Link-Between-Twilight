// ============================================================================
// ww_gfx_spike.cpp - THE GfxService SPIKE. Increment 1 = is the path
// reachable (ANSWERED YES). Increment 2a = can this plugin put PIXELS on
// screen through it (ANSWERED YES, but it drew on a TP stage - now WW-scoped).
// ============================================================================
// ASSIGNMENT: lwood, priority one. The hypothesis under test (History/Bridge
// flagged it as hypothesis, NOT finding): lwood dies on a `fifo::write_data:
// buffer size overflow` inside `aurora::gx::fifo` - THE EXACT PATH A GfxService
// custom draw does not travel. If a WW model reaches the GPU through push_draw
// instead of GX, the runaway may simply not exist, and the plugin gains a
// delivery route that needs no aurora patch at all.
// Design and the three pre-stated outcomes:
//   docs/state/ww-staging/SPIKE-gfxservice-lwood.md
//
// WHY THIS INCREMENT IS DELIBERATELY NOT THE DRAW. Authoring a WGSL pipeline is
// the expensive half of the spike and it is worth nothing if the service is not
// acquirable on this host, the draw type will not register, or the stage hook
// never fires. Those are three cheap yes/no questions, they gate every line
// that would come after them, and ONE BOOT ANSWERS ALL THREE.
//
// EVERY STEP CARRIES ITS OWN RETURN CODE. The spike design names outcome (3),
// "it never drew at all", as the trap: a silent failure is indistinguishable
// from "the hypothesis was wrong" unless each step says whether it ran. A step
// that never ran must never be readable as a step that ran and was fine. That
// exact confusion cost this lane eleven bisect modes and two retractions.
//
// GATED: WW_GFX_SPIKE. Absent or 0 and this file does nothing whatsoever - no
// service call, no registration, no hook. 1 = increment 1 (reachability, zero
// GPU work). 2 = increment 2a (first pixels). Levels are CUMULATIVE, so a
// level-2 run still emits every increment-1 receipt and increment 1's verdict
// stays reproducible. The gate is read once.
//
// AND SCOPED TO WW HOST STAGES. The first level-2 run drew the 2a probe on a TP
// stage, because `register_stage_hook` is global and this file did not check
// where it was. The user caught it. `spikeStage` now returns early unless
// `wwRegistry_isWwHostStage()`, so on TP nothing is pushed, nothing is
// recorded, and no command this plugin owns enters mainline TP's frame.
// ============================================================================
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <mods/api.h>
#include <mods/svc/camera.h>
#include <mods/svc/gfx.h>
#include <mods/svc/log.h>
#include <webgpu/webgpu.h>

#include "registry.h"  // wwRegistry_isWwHostStage()

// ---------------------------------------------------------------------------
// OPTIONAL, NOT REQUIRED - AND THIS IS THE MOST LOAD-BEARING LINE IN THE FILE.
//
// `IMPORT_SERVICE` expands with SERVICE_IMPORT_REQUIRED, and the SDK is explicit
// that a required import the host cannot satisfy makes THE MOD FAIL TO LOAD
// (sdk/include/mods/service.hpp:44). A required GfxService import would
// therefore take the ENTIRE ww_donor_disc plugin offline - the working Outset
// warp, the disc reader, every hook - on any host build that does not provide
// it. A spike must never be able to do that. The optional form yields a null
// pointer instead, which the gate below reports as a receipt and survives.
//
// AND PINNED TO MINOR 0, WHICH IS NOT A DETAIL. There are TWO SDK trees here.
// The plugin compiles against DUSK_ROOT = the clean vanilla checkout, whose
// `gfx.h` declares GFX_SERVICE_MINOR 2; the fork's own tree declares MINOR 0.
// The unversioned `IMPORT_OPTIONAL_SERVICE` requests
// `ServiceTraits<GfxService>::minor_version` — whatever the COMPILING header
// says — so a build made against the vanilla SDK would demand minor 2 and get a
// NULL pointer from any host providing minor 0. The spike would then report
// "the host does not provide GfxService" while the host provides it perfectly
// well: a FALSE outcome (3), which is the exact failure this spike's design
// names as its trap. Every call this file makes — get_device_info,
// register_draw_type, push_draw, register_stage_hook, both unregisters — plus
// every field it reads (`uses_reversed_z`, `sample_count`) and both callback
// signatures are present and IDENTICAL at minor 0, so pinning costs nothing
// and makes the spike answer honestly on either host.
// ---------------------------------------------------------------------------
IMPORT_OPTIONAL_SERVICE_VERSION(GfxService, s_gfxSpikeSvc, 0u);
IMPORT_OPTIONAL_SERVICE(CameraService, s_gfxSpikeCam);

// The plugin's `logf` lives in registry.cpp's ANONYMOUS namespace (opens 1340,
// closes 7453), so it has internal linkage and cannot be called from here - the
// anon-namespace link trap, which compiles clean and fails at link. Mirror the
// epoch-2 idiom registry.cpp itself uses for LogService instead.
#if defined(__has_include) && __has_include(<mods/svc/hook.hpp>)
extern const LogService* s_log;  // epoch 2+: imported once, in main.cpp
#else
IMPORT_SERVICE(LogService, s_spikeLog);
#define s_log s_spikeLog
#endif

namespace {

// ============================================================================
// THE DRAW PAYLOAD - ONE layout, shared by every leg, because the render worker
// decodes it in ONE place. The 128-byte inline cap is nowhere near binding, so
// there is no reason to overlap fields between legs; an earlier version packed
// the vertex count over the box leg's uniform-offset slot and the mesh leg
// silently drew with matrix offset 0 - every tree at the origin, from a bug that
// no receipt would have named.
//   [0]      1 = a WW host stage decided this on the GAME thread
//   [1]      1 = box leg
//   [2]      1 = real-mesh leg
//   [4..7]   uniform range offset  (both legs)
//   [8..11]  vertex range offset   (mesh only)
//   [12..15] vertex range bytes    (mesh only)
//   [16..19] vertex count          (mesh only)
// ============================================================================
enum { WW_PAYLOAD_BYTES = 32 };

int s_enabled = -1;
GfxDrawTypeHandle s_drawType = 0;
GfxStageHookHandle s_stageHook = 0;
int s_stageFires = 0;
int s_drawCalls = 0;

// --- increment 2a state (level 2 only) --------------------------------------
int s_level = 0;                              // 1 = reachability, 2 = first pixels
WGPURenderPipeline s_pipeline = nullptr;
int s_pipelineResult = -1;                    // -1 not attempted, 0 built, 1 module fail, 2 pipeline fail
int s_pixelDraws = 0;
int s_stageSkipped = 0;

// --- increment 2b state: the LWOOD leg -------------------------------------
WGPURenderPipeline s_boxPipeline = nullptr;
WGPUBindGroupLayout s_boxLayout = nullptr;
int s_boxPipelineResult = -1;
CameraInfo s_camera = CAMERA_INFO_INIT;
bool s_cameraValid = false;
int s_lwoodSubmits = 0;      // instances handed to us this frame, cumulative
int s_lwoodDrawn = 0;        // instances actually recorded on the worker
int s_lwoodRejected = 0;     // handed to us but not drawable (no camera/pipeline)

// --- the REAL MESH leg (level 4) --------------------------------------------
WGPURenderPipeline s_meshPipeline = nullptr;
WGPUBindGroupLayout s_meshLayout = nullptr;
int s_meshPipelineResult = -1;
int s_meshDraws = 0;
int s_meshTrisLast = 0;

// ============================================================================
// INCREMENT 2a SHADER — the smallest thing that can put pixels on screen.
//
// NO VERTEX BUFFER AND NO CAMERA ON PURPOSE. The triangle is generated from
// `vertex_index` straight in clip space, so this tests EXACTLY ONE thing: can
// the plugin compile a shader, create a pipeline against the scene pass, and
// record a draw into aurora's own RenderPassEncoder. A vertex buffer would add
// `push_verts` to the same experiment, and an MVP would add CameraService; if
// the draw then failed, three candidates would share one symptom. Increment 2b
// adds geometry and camera ONCE this is known good — the same reason increment
// 1 issued no GPU work at all.
//
// It sits in the top-left corner and is small: unmistakable when present,
// harmless to the frame when the user is looking at anything else.
// ============================================================================
const char* const kSpikeWgsl = R"WGSL(
@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4<f32> {
  var p = array<vec2<f32>, 3>(
    vec2<f32>(-0.95,  0.95),
    vec2<f32>(-0.55,  0.95),
    vec2<f32>(-0.95,  0.55));
  return vec4<f32>(p[idx], 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4<f32> {
  return vec4<f32>(1.0, 0.25, 0.85, 1.0);
}
)WGSL";

// ============================================================================
// INCREMENT 2b - THE ACTUAL LWOOD TEST.
//
// THE QUESTION, restated so the shape of the answer is fixed before the run:
// lwood dies on a `fifo::write_data: buffer size overflow` in `aurora::gx::fifo`
// when submitted through GX. Does the SAME set of trees, at the SAME world
// positions, every frame, survive when delivered through GfxService instead?
//
// WHY A BOX AND NOT THE TREE MESH - AND THIS IS A DELIBERATE SEPARATION, NOT A
// SHORTCUT. J3D does not store triangles: geometry lives in a GX DISPLAY LIST
// inside each J3DShape, so real lwood mesh needs a VCD/VAT decoder that walks
// the primitive stream. That decoder is a large piece of work AND IT IS NOT ON
// THE PATH TO THE ANSWER. What the crash question actually turns on is whether
// per-instance, per-frame draws through this route accumulate the way the GX
// path does. A box at the donor's own cull extents exercises exactly that -
// same instance count, same frame cadence, same camera, real world placement -
// while holding the mesh decoder out of the experiment. If the box survives,
// the route is sound and the decoder is worth building. If the box overflows
// too, the decoder would have been wasted work.
//
// EXTENTS ARE THE DONOR'S OWN, not invented: `fopAcM_setCullSizeBox(this,
// -600, -0, -600, 600, 900, 600)` in d_a_lwood.cpp CreateInit. So each box is
// the volume the donor itself says a tree occupies.
//
// Corners come from `vertex_index` - no vertex buffer, so `push_verts` is not a
// variable in this experiment either. Only the MVP crosses, as a uniform.
// ============================================================================
const char* const kBoxWgsl = R"WGSL(
struct U { mvp: mat4x4<f32>, };
@group(0) @binding(0) var<uniform> u: U;

@vertex
fn vs_main(@builtin(vertex_index) vi: u32) -> @builtin(position) vec4<f32> {
  // 12 triangles, 36 indices, unit cube in [0,1]; scaled to the donor extents.
  var idx = array<u32, 36>(
    0u,2u,1u, 1u,2u,3u,   4u,5u,6u, 5u,7u,6u,
    0u,1u,4u, 1u,5u,4u,   2u,6u,3u, 3u,6u,7u,
    0u,4u,2u, 2u,4u,6u,   1u,3u,5u, 3u,7u,5u);
  let c = idx[vi];
  let unit = vec3<f32>(f32(c & 1u), f32((c >> 1u) & 1u), f32((c >> 2u) & 1u));
  // SHRUNK AND LIFTED CLEAR OF THE GROUND - this is a DISCRIMINATOR, not a
  // cosmetic tweak. The first box used the donor's CULL extents
  // (-600,0,-600 .. 600,900,600), which is a deliberately generous visibility
  // volume, not the tree's silhouette - so it dwarfed the trunk and, more
  // importantly, interpenetrated terrain everywhere. With depth write on and a
  // reversed-Z GREATER compare, that is a textbook z-fighting source and it
  // would look exactly like the flicker the user reported.
  // This box touches nothing: it floats above the ground with no other geometry
  // occupying it. IF THE FLICKER STOPS, it was z-fighting against terrain and
  // says nothing about the GfxService route. IF THE FLICKER PERSISTS, it is the
  // route or the per-frame uniform and MUST be fixed before the mesh decoder is
  // written, because real geometry would inherit it.
  let lo = vec3<f32>( -60.0, 200.0,  -60.0);
  let hi = vec3<f32>(  60.0, 500.0,   60.0);
  let p = lo + unit * (hi - lo);
  return u.mvp * vec4<f32>(p, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4<f32> {
  return vec4<f32>(0.15, 0.9, 0.35, 1.0);
}
)WGSL";

// ============================================================================
// REAL LWOOD MESH - positions from the model's own GX display lists.
//
// Flat-shaded from a cheap height ramp rather than a constant colour: with no
// normals decoded there is no lighting, and a single flat colour makes a tree
// read as a silhouette blob where you cannot tell correct geometry from a
// tangle. A ramp makes the shape legible enough to JUDGE, which is the whole
// point of putting it on screen.
// ============================================================================
const char* const kMeshWgsl = R"WGSL(
struct U { mvp: mat4x4<f32>, };
@group(0) @binding(0) var<uniform> u: U;

struct VOut {
  @builtin(position) pos: vec4<f32>,
  @location(0) h: f32,
};

@vertex
fn vs_main(@location(0) p: vec3<f32>) -> VOut {
  var o: VOut;
  o.pos = u.mvp * vec4<f32>(p, 1.0);
  o.h = clamp(p.y / 600.0, 0.0, 1.0);
  return o;
}

@fragment
fn fs_main(in: VOut) -> @location(0) vec4<f32> {
  let trunk = vec3<f32>(0.35, 0.22, 0.12);
  let leaf  = vec3<f32>(0.20, 0.65, 0.25);
  return vec4<f32>(mix(trunk, leaf, in.h), 1.0);
}
)WGSL";

void spikeLogf(const char* fmt, ...) {
    if (s_log == nullptr) {
        return;
    }
    char buf[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    s_log->write(mod_ctx, LOG_LEVEL_INFO, buf);
}

// WW_GFX_SPIKE: 1 = increment 1 (reachability, no GPU work), 2 = increment 2a
// (first pixels). Levels are cumulative — 2 keeps every increment-1 receipt, so
// increment 1's verdict stays reproducible from a level-2 run.
bool spikeEnabled() {
    if (s_enabled < 0) {
        const char* e = std::getenv("WW_GFX_SPIKE");
        s_level = (e != nullptr && e[0] >= '1' && e[0] <= '9') ? (e[0] - '0') : 0;
        s_enabled = s_level > 0 ? 1 : 0;
    }
    return s_enabled == 1;
}

// ============================================================================
// PIPELINE — built once, from minor-0 API only.
//
// `get_scene_target_layout` is the header's recommended path for scene
// pipelines, and it is DELIBERATELY NOT USED HERE: it is a MINOR 2 member, so on
// a minor-0 host that field does not exist and reading it runs off the end of
// the service struct. The deprecated `GfxDeviceInfo` format fields carry the
// same information, exist at minor 0, and increment 1 already proved this host
// populates them (`reversed_z:1`, `samples:1`). One color target is all a
// clip-space triangle needs; a multi-attachment scene is increment 2b's problem
// and can be version-gated there, against a host whose minor is known.
// ============================================================================
bool buildSpikePipeline(const GfxDeviceInfo& info) {
    WGPUShaderSourceWGSL wgsl = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgsl.code = {kSpikeWgsl, WGPU_STRLEN};
    WGPUShaderModuleDescriptor moduleDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    moduleDesc.nextInChain = &wgsl.chain;
    moduleDesc.label = {"ww_spike", WGPU_STRLEN};
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(info.device, &moduleDesc);
    if (module == nullptr) {
        s_pipelineResult = 1;
        return false;
    }

    WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
    colorTarget.format = info.color_format;
    colorTarget.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = module;
    fragment.entryPoint = {"fs_main", WGPU_STRLEN};
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;

    // Depth ALWAYS, write disabled: the triangle is a visibility probe, not part
    // of the scene. Depth-testing it against terrain would let a failure to draw
    // and a successful draw that is simply occluded look identical.
    WGPUDepthStencilState depthStencil = WGPU_DEPTH_STENCIL_STATE_INIT;
    depthStencil.format = info.depth_format;
    depthStencil.depthWriteEnabled = WGPUOptionalBool_False;
    depthStencil.depthCompare = WGPUCompareFunction_Always;

    WGPURenderPipelineDescriptor desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    desc.label = {"ww_spike", WGPU_STRLEN};
    desc.vertex.module = module;
    desc.vertex.entryPoint = {"vs_main", WGPU_STRLEN};
    desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    desc.depthStencil = &depthStencil;
    desc.multisample.count = info.sample_count;
    desc.fragment = &fragment;

    s_pipeline = wgpuDeviceCreateRenderPipeline(info.device, &desc);
    wgpuShaderModuleRelease(module);
    s_pipelineResult = (s_pipeline != nullptr) ? 0 : 2;
    return s_pipeline != nullptr;
}

// ============================================================================
// LWOOD PIPELINE - same minor-0-only discipline as 2a, with two differences
// that matter and are stated rather than buried:
//   1. DEPTH IS REAL HERE. 2a used compare ALWAYS so a failure to draw could
//      not hide behind occlusion. A world-space box MUST depth-test or it will
//      paint over terrain and tell us nothing about placement. `uses_reversed_z`
//      is 1 on this host (increment 1 measured it), so GREATER is the correct
//      comparison - LESS would silently reject every fragment and look exactly
//      like "the route does not work".
//   2. It has a bind group: one uniform, the per-instance MVP.
// ============================================================================
bool buildBoxPipeline(const GfxDeviceInfo& info) {
    WGPUShaderSourceWGSL wgsl = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgsl.code = {kBoxWgsl, WGPU_STRLEN};
    WGPUShaderModuleDescriptor moduleDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    moduleDesc.nextInChain = &wgsl.chain;
    moduleDesc.label = {"ww_lwood_box", WGPU_STRLEN};
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(info.device, &moduleDesc);
    if (module == nullptr) {
        s_boxPipelineResult = 1;
        return false;
    }

    WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
    colorTarget.format = info.color_format;
    colorTarget.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = module;
    fragment.entryPoint = {"fs_main", WGPU_STRLEN};
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;

    WGPUDepthStencilState depthStencil = WGPU_DEPTH_STENCIL_STATE_INIT;
    depthStencil.format = info.depth_format;
    depthStencil.depthWriteEnabled = WGPUOptionalBool_True;
    depthStencil.depthCompare =
        info.uses_reversed_z ? WGPUCompareFunction_Greater : WGPUCompareFunction_Less;

    WGPURenderPipelineDescriptor desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    desc.label = {"ww_lwood_box", WGPU_STRLEN};
    desc.vertex.module = module;
    desc.vertex.entryPoint = {"vs_main", WGPU_STRLEN};
    desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    desc.primitive.cullMode = WGPUCullMode_None;  // box winding is not worth a bug
    desc.depthStencil = &depthStencil;
    desc.multisample.count = info.sample_count;
    desc.fragment = &fragment;

    s_boxPipeline = wgpuDeviceCreateRenderPipeline(info.device, &desc);
    wgpuShaderModuleRelease(module);
    if (s_boxPipeline == nullptr) {
        s_boxPipelineResult = 2;
        return false;
    }
    s_boxLayout = wgpuRenderPipelineGetBindGroupLayout(s_boxPipeline, 0);
    s_boxPipelineResult = (s_boxLayout != nullptr) ? 0 : 3;
    return s_boxLayout != nullptr;
}

// ============================================================================
// MESH PIPELINE - the first one here that takes a REAL VERTEX BUFFER.
//
// Everything before this generated its corners in the shader, which meant
// `push_verts` was never exercised. That was deliberate while the question was
// "does the route work"; it is exactly the wrong simplification now, because the
// crash under investigation is a DATA VOLUME failure and vertex upload is the
// part that carries volume.
// ============================================================================
bool buildMeshPipeline(const GfxDeviceInfo& info) {
    WGPUShaderSourceWGSL wgsl = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgsl.code = {kMeshWgsl, WGPU_STRLEN};
    WGPUShaderModuleDescriptor moduleDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    moduleDesc.nextInChain = &wgsl.chain;
    moduleDesc.label = {"ww_lwood_mesh", WGPU_STRLEN};
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(info.device, &moduleDesc);
    if (module == nullptr) {
        s_meshPipelineResult = 1;
        return false;
    }

    WGPUVertexAttribute attr = WGPU_VERTEX_ATTRIBUTE_INIT;
    attr.format = WGPUVertexFormat_Float32x3;
    attr.offset = 0;
    attr.shaderLocation = 0;
    WGPUVertexBufferLayout vbl = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
    vbl.arrayStride = sizeof(float) * 3;
    vbl.stepMode = WGPUVertexStepMode_Vertex;
    vbl.attributeCount = 1;
    vbl.attributes = &attr;

    WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
    colorTarget.format = info.color_format;
    colorTarget.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = module;
    fragment.entryPoint = {"fs_main", WGPU_STRLEN};
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;

    WGPUDepthStencilState depthStencil = WGPU_DEPTH_STENCIL_STATE_INIT;
    depthStencil.format = info.depth_format;
    depthStencil.depthWriteEnabled = WGPUOptionalBool_True;
    depthStencil.depthCompare =
        info.uses_reversed_z ? WGPUCompareFunction_Greater : WGPUCompareFunction_Less;

    WGPURenderPipelineDescriptor desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    desc.label = {"ww_lwood_mesh", WGPU_STRLEN};
    desc.vertex.module = module;
    desc.vertex.entryPoint = {"vs_main", WGPU_STRLEN};
    desc.vertex.bufferCount = 1;
    desc.vertex.buffers = &vbl;
    desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    // Culling OFF: the decoder expands strips with alternating winding, and one
    // wrong parity would silently erase half the tree. Winding correctness is a
    // separate question from whether the geometry is right.
    desc.primitive.cullMode = WGPUCullMode_None;
    desc.depthStencil = &depthStencil;
    desc.multisample.count = info.sample_count;
    desc.fragment = &fragment;

    s_meshPipeline = wgpuDeviceCreateRenderPipeline(info.device, &desc);
    wgpuShaderModuleRelease(module);
    if (s_meshPipeline == nullptr) {
        s_meshPipelineResult = 2;
        return false;
    }
    s_meshLayout = wgpuRenderPipelineGetBindGroupLayout(s_meshPipeline, 0);
    s_meshPipelineResult = (s_meshLayout != nullptr) ? 0 : 3;
    return s_meshLayout != nullptr;
}

// Column-major store for WGSL, matching the reference mod's helper.
void mulAndStore(const float projFromWorld[16], const float world34[12], float out[16]) {
    // world34 is a J3D 3x4 ROW-MAJOR matrix; promote to 4x4 row-major first.
    float w[16];
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 4; ++c) {
            w[r * 4 + c] = world34[r * 4 + c];
        }
    }
    w[12] = 0.0f; w[13] = 0.0f; w[14] = 0.0f; w[15] = 1.0f;

    // proj_from_world arrives COLUMN-major from CameraService; read it as such.
    float m[16];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += projFromWorld[k * 4 + r] * w[k * 4 + c];
            }
            m[r * 4 + c] = sum;   // row-major product
        }
    }
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out[c * 4 + r] = m[r * 4 + c];  // -> column-major for WGSL
        }
    }
}

// ---------------------------------------------------------------------------
// THE DRAW CALLBACK - runs on the RENDER WORKER THREAD while the frame encodes.
// Increment 1 issues no GPU work at all; a call landing here is the finding.
// Increment 2 puts the pipeline and the lwood geometry here, unchanged in shape.
// ---------------------------------------------------------------------------
void spikeDraw(ModContext*, const GfxDrawContext* draw_ctx, const void* payload,
               size_t payload_size, void*) {
    s_drawCalls++;
    if (s_drawCalls <= 3 || (s_drawCalls % 600) == 0) {
        spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_draw\",\"n\":%d,\"payload\":%u,"
                  "\"has_pass\":%d,\"has_vbuf\":%d,"
                  "\"reads\":\"THE RENDER WORKER REACHED THIS PLUGIN. Increment 1 issues no GPU "
                  "work; a call here proves register_draw_type plus push_draw carry a payload "
                  "across the thread boundary, which is the precondition for the entire bypass "
                  "hypothesis. has_pass=1 means a live RenderPassEncoder is in hand, which is "
                  "what increment 2 records the lwood draw into\"}",
                  s_drawCalls, (unsigned)payload_size,
                  (int)(draw_ctx != nullptr && draw_ctx->pass != nullptr),
                  (int)(draw_ctx != nullptr && draw_ctx->vertex_buffer != nullptr));
    }

    // --- increment 2a: the actual draw ---------------------------------------
    // Second, INDEPENDENT guard: the game thread already refused to push_draw on
    // a TP stage, so reaching here with payload[0]==0 would mean the scoping
    // above failed. Belt and braces, because the cost of being wrong is drawing
    // WW content into mainline TP - and it reads the flag the GAME THREAD set
    // rather than re-deriving stage state on the render worker.
    const unsigned char* pb = static_cast<const unsigned char*>(payload);
    const bool bigEnough = (pb != nullptr && payload_size >= WW_PAYLOAD_BYTES);
    const bool payloadSaysLwood = (bigEnough && pb[1] == 1);
    const bool payloadSaysMesh = (bigEnough && pb[2] == 1);
    uint64_t uniformOffset = 0, vertOffset = 0;
    uint32_t vertBytes = 0, meshVerts = 0;
    if (bigEnough) {
        uint32_t uo = 0, vo = 0, vb = 0, vc = 0;
        std::memcpy(&uo, pb + 4, sizeof(uo));
        std::memcpy(&vo, pb + 8, sizeof(vo));
        std::memcpy(&vb, pb + 12, sizeof(vb));
        std::memcpy(&vc, pb + 16, sizeof(vc));
        uniformOffset = uo; vertOffset = vo; vertBytes = vb; meshVerts = vc;
    }
    const bool payloadSaysWw =
        (payload != nullptr && payload_size >= 1 &&
         static_cast<const unsigned char*>(payload)[0] == 1);
    // LEVEL 2 ONLY, not >=2: the 2a probe triangle was a VISIBILITY TEST and it
    // passed. Leaving it on during the lwood run (level 3) puts a pink wedge in
    // the corner of every frame for no remaining information. The code stays so
    // `WW_GFX_SPIKE=2` still reproduces the 2a result on demand.
    if (s_level == 2 && payloadSaysWw && s_pipeline != nullptr && draw_ctx != nullptr &&
        draw_ctx->pass != nullptr) {
        wgpuRenderPassEncoderSetPipeline(draw_ctx->pass, s_pipeline);
        wgpuRenderPassEncoderDraw(draw_ctx->pass, 3, 1, 0, 0);
        s_pixelDraws++;
        if (s_pixelDraws == 1) {
            spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_pixels\",\"n\":1,"
                      "\"reads\":\"FIRST RECORDED DRAW. The plugin has set its own pipeline and "
                      "issued draw(3) into aurora's RenderPassEncoder. THIS RECEIPT ONLY PROVES "
                      "THE COMMAND WAS RECORDED, NOT THAT ANYTHING IS VISIBLE - a pink triangle "
                      "in the TOP-LEFT corner is the only proof of that, and only the user can "
                      "see it. Do not report a pass from this line alone\"}");
        }
    }
    // --- level 4: one lwood instance as REAL MESH ----------------------------
    if (payloadSaysMesh && s_meshPipeline != nullptr && s_meshLayout != nullptr &&
        draw_ctx != nullptr && draw_ctx->pass != nullptr &&
        draw_ctx->uniform_buffer != nullptr && draw_ctx->vertex_buffer != nullptr &&
        meshVerts >= 3) {
        WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
        entry.binding = 0;
        entry.buffer = draw_ctx->uniform_buffer;
        entry.offset = uniformOffset;
        entry.size = 64;
        WGPUBindGroupDescriptor bgDesc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
        bgDesc.layout = s_meshLayout;
        bgDesc.entryCount = 1;
        bgDesc.entries = &entry;
        WGPUBindGroup bg = wgpuDeviceCreateBindGroup(draw_ctx->device, &bgDesc);
        if (bg != nullptr) {
            wgpuRenderPassEncoderSetPipeline(draw_ctx->pass, s_meshPipeline);
            wgpuRenderPassEncoderSetBindGroup(draw_ctx->pass, 0, bg, 0, nullptr);
            wgpuRenderPassEncoderSetVertexBuffer(draw_ctx->pass, 0, draw_ctx->vertex_buffer,
                                                 vertOffset, vertBytes);
            wgpuRenderPassEncoderDraw(draw_ctx->pass, meshVerts, 1, 0, 0);
            wgpuBindGroupRelease(bg);
            s_meshDraws++;
            if (s_meshDraws == 1) {
                spikeLogf("[WwRegistry] {\"ev\":\"lwood_mesh_first\",\"verts\":%u,"
                          "\"bytes\":%u,"
                          "\"reads\":\"FIRST REAL LWOOD MESH RECORDED THROUGH GfxService - "
                          "decoded from the model's own GX display lists, uploaded with "
                          "push_verts. THIS is the data-volume test the box could not be: the "
                          "box was 36 shader-generated corners and uploaded nothing\"}",
                          meshVerts, vertBytes);
            }
        }
        return;   // mesh replaces the box for this instance
    }

    // --- increment 2b: one lwood instance ------------------------------------
    // The payload's second byte marks a lwood submission and carries the uniform
    // range the GAME THREAD reserved. Nothing is re-derived here: the render
    // worker never touches game state.
    if (payloadSaysLwood && s_boxPipeline != nullptr && s_boxLayout != nullptr &&
        draw_ctx != nullptr && draw_ctx->pass != nullptr &&
        draw_ctx->uniform_buffer != nullptr) {
        WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
        entry.binding = 0;
        entry.buffer = draw_ctx->uniform_buffer;
        entry.offset = uniformOffset;
        entry.size = 64;
        WGPUBindGroupDescriptor bgDesc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
        bgDesc.layout = s_boxLayout;
        bgDesc.entryCount = 1;
        bgDesc.entries = &entry;
        WGPUBindGroup bg = wgpuDeviceCreateBindGroup(draw_ctx->device, &bgDesc);
        if (bg != nullptr) {
            wgpuRenderPassEncoderSetPipeline(draw_ctx->pass, s_boxPipeline);
            wgpuRenderPassEncoderSetBindGroup(draw_ctx->pass, 0, bg, 0, nullptr);
            wgpuRenderPassEncoderDraw(draw_ctx->pass, 36, 1, 0, 0);
            wgpuBindGroupRelease(bg);
            s_lwoodDrawn++;
            if (s_lwoodDrawn == 1) {
                spikeLogf("[WwRegistry] {\"ev\":\"lwood_gfx_first\",\"n\":1,"
                          "\"reads\":\"FIRST LWOOD INSTANCE RECORDED THROUGH GfxService. This "
                          "is the draw that the GX path could not survive. It proves the command "
                          "was recorded, NOT that a green box is on screen and NOT yet that the "
                          "run completes - the overflow killed GX at draw 44, so the number that "
                          "matters is the shutdown tally after a full Outset visit\"}");
            }
        }
    }
    (void)payload;
}

// ---------------------------------------------------------------------------
// THE STAGE HOOK - game thread, frame recording. SCENE_AFTER_TERRAIN is where
// trees belong, so this is also the stage increment 2 draws at: no relocation.
// ---------------------------------------------------------------------------
void spikeStage(ModContext* ctx, const GfxStageContext* stage_ctx, void*) {
    s_stageFires++;

    // ------------------------------------------------------------------------
    // WW-STAGE SCOPING — NOTHING IS RECORDED ON A TP STAGE. NOT ONE COMMAND.
    //
    // This gate was MISSING on the first level-2 run and the user caught it: the
    // 2a probe triangle drew on a TP stage, which is WW-lane rendering bleeding
    // into mainline TP. `register_stage_hook` is global — it fires for every
    // stage, and a hook that draws unconditionally is a shared-path edit with no
    // runtime scope, the exact shape that has already cost this project two
    // mainline-TP crashes in one night.
    //
    // THE PREDICATE IS EVALUATED HERE, ON THE GAME THREAD, AND DELIBERATELY NOT
    // IN THE DRAW CALLBACK. `spikeDraw` runs on the RENDER WORKER while the
    // frame encodes; calling into game state from there would be a data race
    // even when the answer happened to be right. The game thread decides, and
    // the decision travels to the worker in the payload — which is what the
    // 128-byte inline payload exists for.
    //
    // AND THE GATE IS AN EARLY RETURN, NOT A FLAG THE DRAW CHECKS: on a TP stage
    // `push_draw` is never called at all, so the worker is never handed WW work
    // it must then decline. Refusing late still means the command was queued.
    // ------------------------------------------------------------------------
    const bool wwStage = wwRegistry_isWwHostStage();
    if (!wwStage) {
        s_stageSkipped++;
        if (s_stageSkipped == 1 || (s_stageSkipped % 1800) == 0) {
            spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_scope\",\"skipped\":%d,\"drawn\":%d,"
                      "\"reads\":\"NOT A WW HOST STAGE - push_draw NOT called, so nothing this "
                      "plugin owns is recorded into TP's frame. skipped counting up on a TP "
                      "stage is the gate WORKING, not a fault. If drawn also climbs while on "
                      "TP, the predicate is wrong and that is a bleedover bug\"}",
                      s_stageSkipped, s_pixelDraws);
        }
        return;
    }

    // Camera captured HERE, on the game thread, from the stage context. The
    // draw callback must never ask the game where the camera is.
    if (s_gfxSpikeCam != nullptr && stage_ctx != nullptr && stage_ctx->game_view != nullptr) {
        CameraInfo info = CAMERA_INFO_INIT;
        s_cameraValid = (s_gfxSpikeCam->get_camera(ctx, stage_ctx->game_view, &info) == MOD_OK);
        if (s_cameraValid) {
            s_camera = info;
        }
    }

    unsigned char payload[WW_PAYLOAD_BYTES];
    std::memset(payload, 0, sizeof(payload));
    payload[0] = 1;  // ww-stage confirmed on the game thread; carried, not re-derived
    const ModResult r = s_gfxSpikeSvc->push_draw(ctx, s_drawType, payload, sizeof(payload));
    if (s_stageFires <= 3 || (s_stageFires % 600) == 0) {
        spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_stage\",\"n\":%d,\"stage\":%d,"
                  "\"has_view\":%d,\"push_draw\":%d,\"draws_so_far\":%d,"
                  "\"reads\":\"SCENE_AFTER_TERRAIN fired on the game thread. push_draw is the "
                  "RETURN CODE, 0=MOD_OK; a non-zero here with no gfx_spike_draw line is outcome "
                  "(3) of the spike design - never ran - and must NOT be read as the hypothesis "
                  "failing. has_view=1 means a world camera is available, which increment 2 needs "
                  "for the MVP matrix\"}",
                  s_stageFires, (int)(stage_ctx != nullptr ? (int)stage_ctx->stage : -1),
                  (int)(stage_ctx != nullptr && stage_ctx->game_view != nullptr), (int)r,
                  s_drawCalls);
    }
}

}  // namespace

// ============================================================================
// THE LWOOD ENTRY POINT - called by WwLwood_draw, once per instance per frame.
//
// CALLED FROM registry.cpp AND NOT FROM A STAGE HOOK, on purpose. WwLwood_draw
// only runs for WW lwood actors, so this leg is WW-scoped BY CONSTRUCTION -
// there is no stage predicate to get wrong and no way for it to touch a TP
// frame, which is the failure the user caught on the 2a probe.
//
// `world34` is the model's own base TR matrix - J3D 3x4, row-major - so each box
// lands exactly where that tree stands, with the donor's own extents.
//
// Returns true if the instance was handed to the render worker. FALSE IS NOT AN
// ERROR THE CALLER SHOULD HIDE: it means no camera or no pipeline this frame,
// and the counters say which, so a silent zero can never be read as "drew fine".
// ============================================================================
// ============================================================================
// THE REAL-MESH ENTRY POINT (level 4). Called by WwLwood_draw with geometry the
// caller decoded from the model's own display lists.
//
// `verts` is a flat xyz triangle list in MODEL space; the world matrix places
// it. Uploading per instance per frame is deliberately the NAIVE shape - it is
// the shape that stresses the buffer, and stressing the buffer is the entire
// point. If a per-frame upload of real geometry is what overflows, that is the
// finding, and caching it away would have hidden it.
// ============================================================================
bool wwGfxSpike_submitMesh(const float* verts, int vertCount, const float world34[12]) {
    if (s_enabled != 1 || s_level < 4 || verts == nullptr || vertCount < 3 ||
        world34 == nullptr) {
        return false;
    }
    if (!s_cameraValid || s_meshPipeline == nullptr || s_gfxSpikeSvc == nullptr) {
        s_lwoodRejected++;
        return false;
    }

    float mvp[16];
    mulAndStore(s_camera.proj_from_world, world34, mvp);

    GfxRange uni = {0, 0};
    if (s_gfxSpikeSvc->push_uniform(mod_ctx, mvp, sizeof(mvp), &uni) != MOD_OK) {
        s_lwoodRejected++;
        return false;
    }
    GfxRange vb = {0, 0};
    const size_t bytes = static_cast<size_t>(vertCount) * 3u * sizeof(float);
    if (s_gfxSpikeSvc->push_verts(mod_ctx, verts, bytes, sizeof(float) * 3, &vb) != MOD_OK) {
        s_lwoodRejected++;
        return false;
    }

    unsigned char payload[WW_PAYLOAD_BYTES];
    std::memset(payload, 0, sizeof(payload));
    payload[0] = 1;   // ww stage
    payload[2] = 1;   // real-mesh leg
    const uint32_t uoff = uni.offset;
    const uint32_t voff = vb.offset;
    const uint32_t vbytes = static_cast<uint32_t>(bytes);
    const uint32_t vcount = static_cast<uint32_t>(vertCount);
    std::memcpy(payload + 4, &uoff, sizeof(uoff));
    std::memcpy(payload + 8, &voff, sizeof(voff));
    std::memcpy(payload + 12, &vbytes, sizeof(vbytes));
    std::memcpy(payload + 16, &vcount, sizeof(vcount));
    if (s_gfxSpikeSvc->push_draw(mod_ctx, s_drawType, payload, sizeof(payload)) != MOD_OK) {
        s_lwoodRejected++;
        return false;
    }
    s_lwoodSubmits++;
    s_meshTrisLast = vertCount / 3;
    return true;
}

bool wwGfxSpike_submitLwood(const float world34[12]) {
    if (s_enabled != 1 || s_level < 3 || world34 == nullptr) {
        return false;
    }
    if (!s_cameraValid || s_boxPipeline == nullptr || s_gfxSpikeSvc == nullptr) {
        s_lwoodRejected++;
        if (s_lwoodRejected == 1 || (s_lwoodRejected % 600) == 0) {
            spikeLogf("[WwRegistry] {\"ev\":\"lwood_gfx_reject\",\"n\":%d,\"camera\":%d,"
                      "\"pipeline\":%d,"
                      "\"reads\":\"an lwood instance was offered and NOT submitted. camera=0 "
                      "means the stage hook has not captured a view yet this frame; pipeline=0 "
                      "means the box pipeline failed to build and lwood_gfx_pipeline says why. "
                      "A run with rejects and no draws is NOT evidence about the FIFO\"}",
                      s_lwoodRejected, s_cameraValid ? 1 : 0, s_boxPipeline != nullptr ? 1 : 0);
        }
        return false;
    }

    float mvp[16];
    mulAndStore(s_camera.proj_from_world, world34, mvp);

    GfxRange range = {0, 0};
    if (s_gfxSpikeSvc->push_uniform(mod_ctx, mvp, sizeof(mvp), &range) != MOD_OK) {
        s_lwoodRejected++;
        return false;
    }

    unsigned char payload[WW_PAYLOAD_BYTES];
    std::memset(payload, 0, sizeof(payload));
    payload[0] = 1;  // ww stage
    payload[1] = 1;  // box leg
    const uint32_t off = range.offset;
    std::memcpy(payload + 4, &off, sizeof(off));
    if (s_gfxSpikeSvc->push_draw(mod_ctx, s_drawType, payload, sizeof(payload)) != MOD_OK) {
        s_lwoodRejected++;
        return false;
    }
    s_lwoodSubmits++;
    return true;
}

// ============================================================================
// INIT - three questions, three return codes, one boot.
// ============================================================================
void wwGfxSpike_initialize() {
    if (!spikeEnabled()) {
        return;
    }
    if (s_gfxSpikeSvc == nullptr) {
        spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_init\",\"service\":0,"
                  "\"reads\":\"WW_GFX_SPIKE=1 but THE HOST DOES NOT PROVIDE GfxService. The "
                  "optional import kept the plugin alive rather than failing the load, which a "
                  "required import would have done. The bypass hypothesis cannot be tested on "
                  "this build - that is outcome (3), NOT evidence about lwood\"}");
        return;
    }

    GfxDeviceInfo info = GFX_DEVICE_INFO_INIT;
    const ModResult rDev = s_gfxSpikeSvc->get_device_info(mod_ctx, &info);

    GfxDrawTypeDesc dt = GFX_DRAW_TYPE_DESC_INIT;
    dt.label = "ww_lwood_spike";
    dt.draw = &spikeDraw;
    const ModResult rType = s_gfxSpikeSvc->register_draw_type(mod_ctx, &dt, &s_drawType);

    GfxStageHookDesc sh = GFX_STAGE_HOOK_DESC_INIT;
    sh.callback = &spikeStage;
    const ModResult rHook = s_gfxSpikeSvc->register_stage_hook(
        mod_ctx, GFX_STAGE_SCENE_AFTER_TERRAIN, &sh, &s_stageHook);

    if (s_level >= 2 && rDev == MOD_OK && info.device != nullptr) {
        buildSpikePipeline(info);
        if (s_level >= 3) {
            buildBoxPipeline(info);
            if (s_level >= 4) {
                buildMeshPipeline(info);
            }
        }
        spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_pipeline\",\"result\":%d,\"color_fmt\":%d,"
                  "\"depth_fmt\":%d,\"samples\":%u,"
                  "\"reads\":\"INCREMENT 2a. result 0=pipeline built, 1=shader module failed to "
                  "compile, 2=module compiled but pipeline creation failed. 1 and 2 are DIFFERENT "
                  "failures - 1 is my WGSL, 2 is pipeline state disagreeing with the scene pass "
                  "(format or sample count) - and collapsing them would hide which\"}",
                  s_pipelineResult, (int)info.color_format, (int)info.depth_format,
                  (unsigned)info.sample_count);
        if (s_level >= 3) {
            spikeLogf("[WwRegistry] {\"ev\":\"lwood_gfx_pipeline\",\"result\":%d,"
                      "\"reversed_z\":%d,\"camera_svc\":%d,"
                      "\"reads\":\"THE LWOOD LEG. result 0=ready, 1=WGSL failed, 2=pipeline "
                      "creation failed, 3=pipeline built but bind group layout missing. "
                      "camera_svc=0 means CameraService is absent and every instance will be "
                      "rejected - which would produce a clean run that proves NOTHING about the "
                      "FIFO, so check this line before reading any survival as a result\"}",
                      s_boxPipelineResult, (int)info.uses_reversed_z,
                      s_gfxSpikeCam != nullptr ? 1 : 0);
            if (s_level >= 4) {
                spikeLogf("[WwRegistry] {\"ev\":\"lwood_mesh_pipeline\",\"result\":%d,"
                          "\"reads\":\"THE REAL-MESH LEG. Same result codes as the box leg. "
                          "This is the first pipeline here that takes a VERTEX BUFFER, so a "
                          "failure at 2 points at the vertex layout rather than the shader\"}",
                          s_meshPipelineResult);
            }
        }
    }

    spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_init\",\"service\":1,\"device_info\":%d,"
              "\"draw_type\":%d,\"stage_hook\":%d,\"reversed_z\":%d,\"samples\":%u,"
              "\"reads\":\"THE THREE PRECONDITIONS, one boot, 0=MOD_OK each. Any non-zero means "
              "the GfxService delivery route is unusable on this host and the bypass hypothesis "
              "cannot be tested at all - outcome (3) of the spike design, which must NEVER be "
              "read as the hypothesis being wrong. reversed_z and samples are recorded now "
              "because increment 2 pipeline state must match the scene pass exactly\"}",
              (int)rDev, (int)rType, (int)rHook, (int)info.uses_reversed_z,
              (unsigned)info.sample_count);
}

// ============================================================================
// SHUTDOWN - unregister in reverse. A stage hook outliving the plugin would
// call into unloaded code, which is a crash that would look like lwood's.
// ============================================================================
void wwGfxSpike_shutdown() {
    if (!spikeEnabled() || s_gfxSpikeSvc == nullptr) {
        return;
    }
    if (s_stageHook != 0) {
        s_gfxSpikeSvc->unregister_stage_hook(mod_ctx, s_stageHook);
        s_stageHook = 0;
    }
    if (s_drawType != 0) {
        s_gfxSpikeSvc->unregister_draw_type(mod_ctx, s_drawType);
        s_drawType = 0;
    }
    if (s_pipeline != nullptr) {
        wgpuRenderPipelineRelease(s_pipeline);
        s_pipeline = nullptr;
    }
    if (s_boxLayout != nullptr) {
        wgpuBindGroupLayoutRelease(s_boxLayout);
        s_boxLayout = nullptr;
    }
    if (s_boxPipeline != nullptr) {
        wgpuRenderPipelineRelease(s_boxPipeline);
        s_boxPipeline = nullptr;
    }
    if (s_meshLayout != nullptr) {
        wgpuBindGroupLayoutRelease(s_meshLayout);
        s_meshLayout = nullptr;
    }
    if (s_meshPipeline != nullptr) {
        wgpuRenderPipelineRelease(s_meshPipeline);
        s_meshPipeline = nullptr;
    }
    spikeLogf("[WwRegistry] {\"ev\":\"gfx_spike_shutdown\",\"stage_fires\":%d,\"draws\":%d,"
              "\"level\":%d,\"pipeline\":%d,\"pixel_draws\":%d,\"ww_skipped\":%d,\"lwood_submits\":%d,\"lwood_drawn\":%d,\"lwood_rejected\":%d,\"mesh_draws\":%d,\"mesh_tris\":%d,"
              "\"reads\":\"THE TALLY IS THE VERDICT. stage_fires>0 with draws=0 means the game "
              "thread reached us and the render worker never did. Both >0 means the whole "
              "GfxService route is live. At level 2, pixel_draws>0 means the draw was RECORDED "
              "every frame - it does NOT mean anything was visible, which only the user can "
              "confirm by seeing a pink triangle top-left\"}",
              s_stageFires, s_drawCalls, s_level, s_pipelineResult, s_pixelDraws,
              s_stageSkipped, s_lwoodSubmits, s_lwoodDrawn, s_lwoodRejected,
              s_meshDraws, s_meshTrisLast);
}
