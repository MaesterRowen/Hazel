#include "EditorLayer.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm\glm\gtx\quaternion.hpp>

// Include Scripts
#include "NativeScripts/CameraController.h"

namespace Hazel {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
	{
	}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_SpriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

		m_GrassSprite = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128.0f, 128.0f });

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		// Entity
		m_SquareEntity = m_ActiveScene->CreateEntity("Green square");
		auto& sr = m_SquareEntity.AddComponent<SpriteRendererComponent>( glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
		sr.Sprite = m_GrassSprite;



		m_CameraEntity = m_ActiveScene->CreateEntity("Main Camera");
		m_CameraEntity.AddComponent<CameraComponent>();
		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();


		m_SecondCamera = m_ActiveScene->CreateEntity("Second Camera");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;
	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		HZ_PROFILE_FUNCTION();

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		}

		// Update
		if (m_ViewportFocused)
			m_CameraController.OnUpdate(ts);

		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		
		// Update Scene
		m_ActiveScene->OnUpdate(ts);

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		HZ_PROFILE_FUNCTION();

		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());


		ImGui::Separator();
		if (m_SquareEntity)
		{
			ImGui::Text("%s", m_SquareEntity.GetComponent<TagComponent>().Tag.c_str());

			auto& sr = m_SquareEntity.GetComponent<SpriteRendererComponent>();
			auto& tr = m_SquareEntity.GetComponent<TransformComponent>();
			ImGui::ColorEdit4("Square Color", glm::value_ptr(sr.Color));
			

			bool transformChanged = false;

			// Extracte the translation
			if (ImGui::DragFloat3("Position", glm::value_ptr(tr.Transform[3])))
				transformChanged = true;

			// Extract the scale
			glm::vec3 scale = { glm::length(tr.Transform[0]), glm::length(tr.Transform[1]), glm::length(tr.Transform[2]) };
			if (ImGui::DragFloat3("Scale", glm::value_ptr(scale)))
				transformChanged = true;

			// Extract the rotation
			glm::mat4 temp = tr.Transform;
			temp[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
			
			// Create rotation matrix
			temp[0].x /= scale.x;		temp[0].y /= scale.x;		temp[0].z /= scale.x;
			temp[1].x /= scale.y;		temp[1].y /= scale.y;		temp[1].z /= scale.y;
			temp[2].x /= scale.z;		temp[2].y /= scale.z;		temp[2].z /= scale.z;

			glm::quat quatRotation = glm::quat_cast(temp);
			glm::vec3 eulerAngles = glm::eulerAngles(quatRotation) *180.0f / glm::pi<float>();

			glm::mat4 rotMtx = glm::mat4(1.0f);
			if (ImGui::InputFloat3("Rotation", glm::value_ptr(eulerAngles))) {
				//HZ_CORE_INFO("Temp[0].x: {0}", temp[0].x);
				glm::quat t = glm::quat(glm::vec3(eulerAngles.x, eulerAngles.y, eulerAngles.z));
				HZ_CORE_INFO("Angles: {0}, {1}, {2}", eulerAngles.x, eulerAngles.y, eulerAngles.z);
			
				//glm::quat newRotation = glm::toQuat(glm::orientate4(eulerAngles));
				rotMtx = glm::mat4_cast(t);
				transformChanged = true;
			}

			if (transformChanged) {

				tr.Transform = glm::translate(glm::mat4(1.0f), { tr.Transform[3].x, tr.Transform[3].y, tr.Transform[3].z }) *
					rotMtx * 
					glm::scale(glm::mat4(1.0f), { scale.x, scale.y, scale.z });
			}


			ImGui::InputFloat4("Row 1", glm::value_ptr(tr.Transform[0]));
			ImGui::InputFloat4("Row 2", glm::value_ptr(tr.Transform[1]));
			ImGui::InputFloat4("Row 3", glm::value_ptr(tr.Transform[2]));
			ImGui::InputFloat4("Row 4", glm::value_ptr(tr.Transform[3]));

			ImGui::Separator();
		}

		ImGui::DragFloat3("Camera Transform",
			glm::value_ptr(m_CameraEntity.GetComponent<TransformComponent>().Transform[3]));

		if (ImGui::Checkbox("Camera A", &m_PrimaryCamera))
		{
			m_CameraEntity.GetComponent<CameraComponent>().Primary = m_PrimaryCamera;
			m_SecondCamera.GetComponent<CameraComponent>().Primary = !m_PrimaryCamera;
		}

		{
			auto& camera = m_SecondCamera.GetComponent<CameraComponent>().Camera;
			float orthoSize = camera.GetOrthographicSize();
			if (ImGui::DragFloat("Second Camera Ortho Size", &orthoSize))
			{
				camera.SetOrthographicSize(orthoSize);
			}

		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		//m_CameraController.OnEvent(e);
	}

}