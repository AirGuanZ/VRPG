#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Misc/CascadeShadowMapping.h>

VRPG_GAME_BEGIN

CascadeShadowMapping::CascadeShadowMapping()
    : cascadeZLimit_{ 1, 1, 1 }
{
    nearSM_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[0], GLOBAL_CONFIG.SHADOW_MAP.resolution[0]);
    middleSM_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[1], GLOBAL_CONFIG.SHADOW_MAP.resolution[1]);
    farSM_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[2], GLOBAL_CONFIG.SHADOW_MAP.resolution[2]);
}

void CascadeShadowMapping::RenderShadow(const Camera &camera, const ChunkRenderer &chunkRenderer)
{
    UpdateViewProj(camera, viewProj_, cascadeZLimit_);

    nearSM_->Begin();
    chunkRenderer.RenderShadow({ viewProj_[0] });
    nearSM_->End();

    middleSM_->Begin();
    chunkRenderer.RenderShadow({ viewProj_[1] });
    middleSM_->End();

    farSM_->Begin();
    chunkRenderer.RenderShadow({ viewProj_[2] });
    farSM_->End();
}

void CascadeShadowMapping::FillForwardParams(BlockForwardRenderParams &params)
{
    params.shadowScale = 0.2f;
    params.sunlightDirection = Vec3(5, 6, 7).normalize();

    params.cascadeShadowMaps[0].shadowMapSRV = nearSM_->GetSRV();
    params.cascadeShadowMaps[1].shadowMapSRV = middleSM_->GetSRV();
    params.cascadeShadowMaps[2].shadowMapSRV = farSM_->GetSRV();

    params.cascadeShadowMaps[0].PCFStep = 1.0f / GLOBAL_CONFIG.SHADOW_MAP.resolution[0];
    params.cascadeShadowMaps[1].PCFStep = 1.0f / GLOBAL_CONFIG.SHADOW_MAP.resolution[1];
    params.cascadeShadowMaps[2].PCFStep = 1.0f / GLOBAL_CONFIG.SHADOW_MAP.resolution[2];

    params.cascadeShadowMaps[0].shadowViewProj = viewProj_[0];
    params.cascadeShadowMaps[1].shadowViewProj = viewProj_[1];
    params.cascadeShadowMaps[2].shadowViewProj = viewProj_[2];

    params.cascadeShadowMaps[0].homZLimit = cascadeZLimit_[0];
    params.cascadeShadowMaps[1].homZLimit = cascadeZLimit_[1];
    params.cascadeShadowMaps[2].homZLimit = cascadeZLimit_[2];
}

void CascadeShadowMapping::UpdateViewProj(const Camera &camera, Mat4 viewProj[3], float cascadeZLimit[3])
{
    Vec3 cameraPosition = camera.GetPosition();
    float FOVy = camera.GetFOVy();
    float wOverH = camera.GetWOverH();

    Mat4 shadowView = Trans4::look_at(
        cameraPosition + 500.0f * Vec3(5, 6, 7).normalize(),
        cameraPosition, Vec3(0, 1, 0));
    Mat4 viewToShadow = camera.GetViewMatrix().inv() * shadowView;

    auto constructSingleShadowMapVP = [&](float nearD, float farD)
    {
        float nearYOri = nearD * std::tan(FOVy / 2);
        float nearXOri = wOverH * nearYOri;
        Vec3 nearPoints[4] =
        {
            { +nearXOri, +nearYOri, nearD },
            { +nearXOri, -nearYOri, nearD },
            { -nearXOri, +nearYOri, nearD },
            { -nearXOri, -nearYOri, nearD }
        };

        float farYOri = farD * std::tan(FOVy / 2);
        float farXOri = wOverH * farYOri;
        Vec3 farPoints[4] =
        {
            { +farXOri, +farYOri, farD },
            { +farXOri, -farYOri, farD },
            { -farXOri, +farYOri, farD },
            { -farXOri, -farYOri, farD }
        };

        for(int i = 0; i < 4; ++i)
        {
            Vec4 vn = Vec4(nearPoints[i].x, nearPoints[i].y, nearPoints[i].z, 1);
            nearPoints[i] = (vn * viewToShadow).xyz();

            Vec4 vf = Vec4(farPoints[i].x, farPoints[i].y, farPoints[i].z, 1);
            farPoints[i] = (vf * viewToShadow).xyz();
        }

        float minX = (std::numeric_limits<float>::max)();
        float minY = (std::numeric_limits<float>::max)();
        float minZ = (std::numeric_limits<float>::max)();
        float maxX = (std::numeric_limits<float>::lowest)();
        float maxY = (std::numeric_limits<float>::lowest)();
        float maxZ = (std::numeric_limits<float>::lowest)();

        for(int i = 0; i < 4; ++i)
        {
            minX = (std::min)(minX, nearPoints[i].x);
            minY = (std::min)(minY, nearPoints[i].y);
            minZ = (std::min)(minZ, nearPoints[i].z);
            maxX = (std::max)(maxX, nearPoints[i].x);
            maxY = (std::max)(maxY, nearPoints[i].y);
            maxZ = (std::max)(maxZ, nearPoints[i].z);

            minX = (std::min)(minX, farPoints[i].x);
            minY = (std::min)(minY, farPoints[i].y);
            minZ = (std::min)(minZ, farPoints[i].z);
            maxX = (std::max)(maxX, farPoints[i].x);
            maxY = (std::max)(maxY, farPoints[i].y);
            maxZ = (std::max)(maxZ, farPoints[i].z);
        }

        Mat4 shadowProj = Trans4::orthographic(
            minX - 5, maxX + 5, maxY + 5, minY - 5, 300, maxZ + 5);

        return shadowView * shadowProj;
    };

    float distance0 = camera.GetNearDistance();
    float distance1 = GLOBAL_CONFIG.SHADOW_MAP.distance;
    float distance2 = distance1 + 2 * (distance1 - distance0);
    float distance3 = distance2 + 4 * (distance2 - distance1);

    auto computeHomZLimit = [proj = camera.GetProjMatrix()](float maxZ)
    {
        Vec4 clipV = Vec4(0, 0, maxZ, 1) * proj;
        return clipV.z;
    };

    viewProj[0] = constructSingleShadowMapVP(distance0, distance1);
    viewProj[1] = constructSingleShadowMapVP(distance1, distance2);
    viewProj[2] = constructSingleShadowMapVP(distance2, distance3);

    cascadeZLimit[0] = computeHomZLimit(distance1);
    cascadeZLimit[1] = computeHomZLimit(distance2);
    cascadeZLimit[2] = computeHomZLimit(distance3);
}

VRPG_GAME_END
