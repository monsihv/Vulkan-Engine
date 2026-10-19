struct VecInput {
    float3 position;
    float3 color;
};

struct VecOutput {
    float4 position : SV_Position;
    float3 color;
};

struct Camera {
    column_major float4x4 view;
    column_major float4x4 proj;
};

struct PushConstants {
    column_major float4x4 model;
    Camera *pCameraBuffer;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("vertex")]
VecOutput vertMain(VecInput input) {
    VecOutput output;

    Camera *pCamera = pc.pCameraBuffer;
    output.position = float4(input.position, 1);
    output.position = mul(pCamera->proj, mul(pCamera->view, mul(pc.model, output.position)));

    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 fragMain(VecOutput input) : SV_Target {
    float4 output = float4(input.color, 0);

    return output;
}
