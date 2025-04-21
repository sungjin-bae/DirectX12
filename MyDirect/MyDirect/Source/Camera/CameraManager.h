#pragma once

#include <stdint.h>

#include "../Common/Singleton.h"
#include "Camera.h"
#include <unordered_map>

class CameraManager : public Singleton<CameraManager>
{
public:
    CameraManager();
    virtual ~CameraManager();

    enum class EStackID : uint8_t
    {
        Main,
        MAX
    };

    bool AddCameraForStack(EStackID in_stack_id, uint8_t in_order, const CameraSharedPtr &in_cam);
    bool RemoveCameraForStack(EStackID in_stack_id, uint8_t in_order);

    void Clear();
    void Clear(EStackID in_stack_id, uint8_t in_order);

    auto GetCameras(EStackID in_stack_id) -> const std::unordered_map<uint8_t, CameraSharedPtr>&;

private:
    std::unordered_map<EStackID, std::unordered_map<uint8_t /* order */, CameraSharedPtr>> m_stack;
};
