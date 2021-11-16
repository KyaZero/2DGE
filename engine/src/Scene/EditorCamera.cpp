#include "EditorCamera.h"

namespace Wraith
{
    EditorCamera::EditorCamera(f32 fov, f32 aspect_ratio, f32 near_clip, f32 far_clip)
        : m_Fov(fov)
        , m_AspectRatio(aspect_ratio)
        , m_NearClip(near_clip)
        , m_FarClip(far_clip)
    {
        m_Eye = { 0, 0, -10.0f };
        m_TargetLookAt = { 0, 0, 0 };
        m_Up = { 0, 1, 0 };
        UpdateView();
    }

    void EditorCamera::Update(f32 dt, bool accept_input)
    {
        m_Input.SetEnabled(accept_input);
        auto mouse_pos = m_Input.GetMousePos();

        // left alt
        if (m_Input.IsDown(MouseButton::Right))
            m_CurrentState = CameraState::FreeCam;
        else
            m_CurrentState = CameraState::ArcBall;

        m_CurrentState == CameraState::ArcBall ? UpdateArcball(dt, mouse_pos) : UpdateFreeCam(dt, mouse_pos);

        if (m_Input.IsScrolling() && m_CurrentState != CameraState::FreeCam)
        {
            m_TargetDistance += -(m_Input.GetScrollState().y_offset * ((m_Eye - m_CurrentLookAt).Length() / 20.0f));
        }

        // More snappy for scrolling etc.
        f32 distance_speed = 12.0f;
        m_CurrentDistance = Lerp(m_CurrentDistance, m_TargetDistance, dt * distance_speed);

        f32 transition_speed = 8.0f;
        m_CurrentLookAt = Lerp(m_CurrentLookAt, m_TargetLookAt, dt * transition_speed);

        m_LastMousePos.x = mouse_pos.x;
        m_LastMousePos.y = mouse_pos.y;

        UpdateView();
    }

    void EditorCamera::SetTarget(Vec3f target, f32 scale_magnitude)
    {
        m_TargetLookAt = target;
        m_TargetDistance = 4.0f * scale_magnitude;
        m_Eye = m_CurrentLookAt - GetForward() * m_CurrentDistance;
        UpdateView();
    }

    CameraState EditorCamera::GetCameraState() const { return m_CurrentState; }

    void EditorCamera::UpdateArcball(f32 dt, Vec2f mouse_pos)
    {
        if (m_Input.IsDown(Key::LeftAlt) && m_Input.IsDown(MouseButton::Left))
        {
            Vec4f position = Vec4f(m_Eye, 1.0f);
            Vec4f pivot = Vec4f(m_TargetLookAt, 1.0f);

            f32 delta_x = (2 * PI / m_ViewportSize.x);
            f32 delta_y = (PI / m_ViewportSize.y);
            f32 x_angle = -(m_LastMousePos.x - mouse_pos.x) * delta_x;
            f32 y_angle = (m_LastMousePos.y - mouse_pos.y) * delta_y;

            f32 cos_angle = GetForward().Dot(m_Up);
            if (cos_angle * Sign(delta_y) > 0.99f)
            {
                delta_y = 0;
            }

            Mat4f x_rotation;
            x_rotation = Mat4f::Rotate(x_rotation, x_angle, m_Up);
            position = ((position - pivot) * x_rotation) + pivot;

            Mat4f y_rotation;
            y_rotation = Mat4f::Rotate(y_rotation, y_angle, GetRight());
            Vec4f final_position = ((position - pivot) * y_rotation) + pivot;

            m_Eye = final_position.xyz;
        }
    }

    void EditorCamera::UpdateFreeCam(f32 dt, Vec2f mouse_pos)
    {
        if (m_Input.IsDown(Key::W))
        {
            //TODO: implement
        }
    }

    void EditorCamera::UpdateProjection()
    {
        m_AspectRatio = m_ViewportSize.x / m_ViewportSize.y;
        m_Projection = Mat4f::CreatePerspectiveProjection(m_Fov, -m_AspectRatio, m_NearClip, m_FarClip);
    }

    void EditorCamera::UpdateView()
    {
        m_ViewMatrix = Mat4f::CreateLookAt(m_CurrentLookAt, m_Eye, m_Up);
        m_Eye = m_CurrentLookAt - GetForward() * m_CurrentDistance;
    }
}  // namespace Wraith