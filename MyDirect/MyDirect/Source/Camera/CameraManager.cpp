#include "CameraManager.h"

CameraManager::CameraManager() {}
CameraManager::~CameraManager() {}


bool CameraManager::AddCameraForStack(EStackID in_stack_id, uint8_t in_order, const CameraSharedPtr& in_cam)
{
    if (in_cam == nullptr)
        return false;

    // 값이 이미 존재한다면 덮어씌워짐
    m_stack[in_stack_id][in_order] = in_cam;
}

bool CameraManager::RemoveCameraForStack(EStackID in_stack_id, uint8_t in_order)
{
    auto stack_id_itr = m_stack.find(in_stack_id);
    if (stack_id_itr == m_stack.end())
        return false;

    auto& order_id_map = stack_id_itr->second;
    auto order_id_itr = order_id_map.find(in_order);
    if (order_id_itr == order_id_map.end())
        return false;

    auto result = order_id_map.erase(order_id_itr);
    return result->first;
}

void CameraManager::Clear()
{
    m_stack.clear();
}

void CameraManager::Clear(EStackID in_stack_id, uint8_t in_order)
{
    auto stack_id_itr = m_stack.find(in_stack_id);
    if (stack_id_itr == m_stack.end())
        return;

    auto& order_id_map = stack_id_itr->second;
    auto order_id_itr = order_id_map.find(in_order);
    if (order_id_itr == order_id_map.end())
        return;

    order_id_map.erase(order_id_itr);
}
