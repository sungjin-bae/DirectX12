#ifndef MYDIRECT_SOURCE_CAMERA_CAMERA_MANAGER_H
#define MYDIRECT_SOURCE_CAMERA_CAMERA_MANAGER_H

#include <stdint.h>

#include "Camera.h"
#include "../Common/Singleton.h"

class CameraManager : public Singleton<CameraManager>
{
public:
    CameraManager();
    virtual ~CameraManager();

    enum class EStackID : uint8_t
    {
        Main
    };

    bool AddCameraForStack(EStackID in_stack_id, uint8_t in_order, const CameraSharedPtr& in_cam);
    bool RemoveCameraForStack(EStackID in_stack_id, uint8_t in_order);

    void Clear();
    void Clear(EStackID in_stack_id, uint8_t in_order);
private:
    std::unordered_map<EStackID, std::unordered_map<uint8_t /* order */, CameraSharedPtr>> m_stack;
};

#endif  // MYDIRECT_SOURCE_CAMERA_CAMERA_MANAGER_H
