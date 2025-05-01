#include "../Common/MathHelper.h"

#include "RenderObject.h"

// 정적 멤버 변수 정의
Microsoft::WRL::ComPtr<ID3DBlob> RenderObject::m_vs_byte_code = nullptr;
Microsoft::WRL::ComPtr<ID3DBlob> RenderObject::m_ps_byte_code = nullptr;
std::vector<D3D12_INPUT_ELEMENT_DESC> RenderObject::m_input_layout;

RenderObject::RenderObject(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale)
    : ObjectBase(in_pos, in_rot, in_rot)
    , m_world_matrix(DirectX::XMMatrixIdentity())
    , m_scale_matrix(DirectX::XMMatrixIdentity())
    , m_rotation_matrix(DirectX::XMMatrixIdentity())
    , m_translation_matrix(DirectX::XMMatrixIdentity())
    , m_need_update(true)
{
    static long id = 1;
    m_obj_id = id++;

    // 셰이더와 입력 레이아웃이 아직 초기화되지 않았다면 초기화
    if (m_vs_byte_code == nullptr)
    {
        m_vs_byte_code = d3dUtil::CompileShader(L"..\\..\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
        m_ps_byte_code = d3dUtil::CompileShader(L"..\\..\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

        m_input_layout = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
    }

    // 박스 지오메트리 생성
    CreateBoxGeometry();

    // PSO 설정
    BuildPSODesc();
}

void RenderObject::BuildPSODesc()
{
    ZeroMemory(&m_pso_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    
    m_pso_desc.InputLayout = { m_input_layout.data(), (UINT)m_input_layout.size() };
    m_pso_desc.pRootSignature = nullptr; // Renderer에서 설정
    
    m_pso_desc.VS = 
    { 
        reinterpret_cast<BYTE*>(m_vs_byte_code->GetBufferPointer()), 
        m_vs_byte_code->GetBufferSize() 
    };
    
    m_pso_desc.PS = 
    { 
        reinterpret_cast<BYTE*>(m_ps_byte_code->GetBufferPointer()), 
        m_ps_byte_code->GetBufferSize() 
    };
    
    m_pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    m_pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    m_pso_desc.SampleMask = UINT_MAX;
    m_pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_pso_desc.NumRenderTargets = 1;
    m_pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_pso_desc.SampleDesc.Count = 1;
    m_pso_desc.SampleDesc.Quality = 0;
    m_pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void RenderObject::CreateBoxGeometry()
{
    // 정점과 인덱스 데이터 초기화
    static std::array<Vertex, NUM_VERTICES> vertices = {
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
        Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
    };

    static std::array<std::uint16_t, NUM_INDICES> indices = {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    // 메시 지오메트리 초기화
    const UINT vb_byte_size = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ib_byte_size = (UINT)indices.size() * sizeof(std::uint16_t);

    m_box_geometry = std::make_unique<MeshGeometry>();
    m_box_geometry->Name = "boxGeo";

    AssertHRESULT(D3DCreateBlob(vb_byte_size, &m_box_geometry->VertexBufferCPU));
    CopyMemory(m_box_geometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vb_byte_size);

    AssertHRESULT(D3DCreateBlob(ib_byte_size, &m_box_geometry->IndexBufferCPU));
    CopyMemory(m_box_geometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ib_byte_size);

    // GPU 버퍼는 Renderer에서 생성해야 하므로 여기서는 CPU 버퍼만 초기화
    m_box_geometry->VertexByteStride = sizeof(Vertex);
    m_box_geometry->VertexBufferByteSize = vb_byte_size;
    m_box_geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
    m_box_geometry->IndexBufferByteSize = ib_byte_size;

    SubmeshGeometry submesh;
    submesh.IndexCount = (UINT)indices.size();
    submesh.StartIndexLocation = 0;
    submesh.BaseVertexLocation = 0;

    m_box_geometry->DrawArgs["box"] = submesh;
}

RenderObject::~RenderObject()
{

}

void RenderObject::Update()
{
    if (m_need_update == false)
        return;

    // 스케일 변환
    m_scale_matrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    
    // 회전 변환 (라디안으로 변환)
    m_rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(m_rotation.x),
        DirectX::XMConvertToRadians(m_rotation.y),
        DirectX::XMConvertToRadians(m_rotation.z)
    );
    
    // 위치 변환
    m_translation_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    
    // 변환 행렬들을 결합 (스케일 -> 회전 -> 이동 순서)
    m_world_matrix = m_scale_matrix * m_rotation_matrix * m_translation_matrix;
}

void RenderObject::Render() 
{

}


void RenderObject::OnChanged()
{
    m_need_update = true;
}