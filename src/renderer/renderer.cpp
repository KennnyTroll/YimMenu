#include "renderer.hpp"

#include "file_manager.hpp"
#include "fonts/fonts.hpp"
#include "gui.hpp"
#include "pointers.hpp"

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#include <imgui_internal.h>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
std::string Get_Patch_WFormat(std::string Images_R_path)
{
	char Images_R_cPath[MAX_PATH];
	strcpy_s(Images_R_cPath, (char*)Images_R_path.c_str());
	//LOG(INFO) << "---------> Images_R_cPath:   " << Images_R_cPath;

	char Images_R_cPath_Out[MAX_PATH];
	memset(Images_R_cPath_Out, 0, sizeof(Images_R_cPath_Out));

	int ad_plus = 0;
	for (size_t i = 0; Images_R_cPath[i] != '\0'; i++)
	{
		if ((Images_R_cPath[i] == '\\') || (Images_R_cPath[i] == '/'))
		{
			//LOG(INFO) << "fnd = " << i;
			Images_R_cPath_Out[i + ad_plus] = Images_R_cPath[i];
			ad_plus += 1;
			Images_R_cPath_Out[i + ad_plus] = Images_R_cPath[i];
		}
		else
			Images_R_cPath_Out[i + ad_plus] = Images_R_cPath[i];
	}

	//LOG(INFO) << "--> Images_R_cPath_Out = " << Images_R_cPath_Out;
	std::string Images_R_Path = Images_R_cPath_Out;
	return Images_R_Path;
}

namespace big
{
	renderer::renderer() :
	    m_font_mgr()
	{
	}

	bool renderer::init()
	{
		if (!g_pointers->m_gta.m_swapchain || !*g_pointers->m_gta.m_swapchain)
		{
			LOG(FATAL) << "Invalid swapchain ptr";

			return false;
		}
		m_dxgi_swapchain = *g_pointers->m_gta.m_swapchain;

		if (m_dxgi_swapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_d3d_device)) < 0)
		{
			LOG(FATAL) << "Failed to get D3D device.";

			return false;
		}
		m_d3d_device->GetImmediateContext(&m_d3d_device_context);

		auto file_path = g_file_manager.get_project_file("./imgui.ini").get_path();

		ImGuiContext* ctx = ImGui::CreateContext();

		static std::string path = file_path.make_preferred().string();
		ctx->IO.IniFilename     = path.c_str();

		ImGui_ImplDX11_Init(m_d3d_device, m_d3d_device_context);
		ImGui_ImplWin32_Init(g_pointers->m_hwnd);


		LOG(VERBOSE) << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

		////Initialise le Nombre d images limite pour le rendu
		blip_P_textures_number_total = 0; //Ne pas modifier (incrementation auto)

		////Initialise le Index pour le rendu d'image
		//blip_P_texture_number = 0; //Ne pas modifier (incrementation auto)

		//Dossier dans lequel sont contenus les images
		std::string Imagegif_Folder_Name = "Textures_Pers\\blips_strips"; //A modifier a la valeur voulue
		//Initialise le Vitesse du rendu en nombre d'images secondes
		//gif_P_texture_Img_s_speed = 35;//A modifier a la valeur voulue

		LoadTextureFrom_Gif_Files_Folder(Imagegif_Folder_Name, blip_P_textures, &blip_P_textures_number_total);

		return true;
	}

	void renderer::destroy()
	{
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui::DestroyContext();
	}

	bool renderer::add_dx_callback(dx_callback callback, uint32_t priority)
	{
		if (!m_dx_callbacks.insert({priority, callback}).second)
		{
			LOG(WARNING) << "Duplicate priority given on DX Callback!";

			return false;
		}
		return true;
	}

	void renderer::add_wndproc_callback(wndproc_callback callback)
	{
		m_wndproc_callbacks.emplace_back(callback);
	}

	void renderer::on_present()
	{
		if (m_font_mgr.can_use()) [[likely]]
		{
			new_frame();
			for (const auto& cb : m_dx_callbacks | std::views::values)
				cb();
			end_frame();

			m_font_mgr.release_use();
		}
	}

	void renderer::rescale(float rel_size)
	{
		pre_reset();
		g_gui->restore_default_style();

		if (rel_size != 1.0f)
			ImGui::GetStyle().ScaleAllSizes(rel_size);

		ImGui::GetStyle().MouseCursorScale = 1.0f;
		ImGui::GetIO().FontGlobalScale     = rel_size;
		post_reset();
	}

	void renderer::pre_reset()
	{
		ImGui_ImplDX11_InvalidateDeviceObjects();
	}

	void renderer::post_reset()
	{
		ImGui_ImplDX11_CreateDeviceObjects();
	}

	void renderer::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		for (const auto& cb : m_wndproc_callbacks)
			cb(hwnd, msg, wparam, lparam);


		ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	}

	void renderer::new_frame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void renderer::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	// Simple helper function to load an image into a DX11 texture with common settings
	bool renderer::LoadTextureFromFile(const char* file_Path_name, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
	{
		// C est sert a recuperer La taille de l image Actuel
		int image_width  = 0;
		int image_height = 0;

		//unsigned char* image_data = stbi_load("E:\\Prog_23\\10_mes_build\\GTA\\BBV2_PPM_001\\BigBaseV3PPM\\build\\Debug\\Images\\16.png", &image_width, &image_height, NULL, 4);
		unsigned char* image_data = stbi_load(file_Path_name, &image_width, &image_height, NULL, 4);
		//unsigned char* image_data = stbi__load_main		
		if (image_data == NULL)
		{
			LOG(FATAL) << "--> stbi_load : image_data == NULL";
			return false;
		}

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width            = image_width;
		desc.Height           = image_height;
		desc.MipLevels        = 1;
		desc.ArraySize        = 1;
		desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage            = D3D11_USAGE_DEFAULT;
		desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags   = 0;

		ID3D11Texture2D* pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem          = image_data;
		subResource.SysMemPitch      = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		HRESULT hr                   = m_d3d_device->CreateTexture2D(&desc, &subResource, &pTexture);
		//m_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
		if (FAILED(hr))
		{
			LOG(FATAL) << "--> m_d3d_device->CreateTexture2D : FAILED(hr) " << file_Path_name;
			stbi_image_free(image_data);
			return false;
		}

		if (pTexture == NULL)
		{
			LOG(FATAL) << "--> m_d3d_device->CreateTexture2D : pTexture == NULL " << file_Path_name;
			stbi_image_free(image_data);
			return false;
		}

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels       = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		hr                                = m_d3d_device->CreateShaderResourceView(pTexture, &srvDesc, out_srv);

		// pTexture->Release();
		if (FAILED(hr))
		{
			LOG(FATAL) << "--> m_d3d_device->CreateShaderResourceView : FAILED(hr) " << file_Path_name;
			pTexture->Release();
			return false;
		}

		*out_width  = image_width;
		*out_height = image_height;
		stbi_image_free(image_data);

		return true;
	}

	bool renderer::LoadTextureFromFile_S(std::string& Image_Name_s , ID3D11ShaderResourceView** ID3D11ShaderResource_View)
	{
		auto Images_path = std::filesystem::path(menu_DLL_Path);
		Images_path /= Image_Name_s;
		if (std::filesystem::exists(Images_path))
		{
			std::string Images_path_s = Get_Patch_WFormat(Image_Name_s);
			//LOG(INFO) << "Test to Load " << Image_Name_s << " at : " << Images_path_s;

			// C'est sert a recuperer La taille de l'image Actuel Si besoin
			int my_image_width  = 0;
			int my_image_height = 0;

			bool ret = LoadTextureFromFile(Images_path_s.c_str(), ID3D11ShaderResource_View, &my_image_width, &my_image_height);
			if (!ret)
			{
				//LOG(FATAL) << "LoadTextureFromFile Failed do Load: Images/16.png";
				LOG(FATAL) << "LoadTextureFromFile Failed do Load: " << Image_Name_s;
				return false;
			}
			else
			{
				IM_ASSERT(ret);
				LOG(INFO) << Image_Name_s << " Loaded, Size " << my_image_width << " X " << my_image_height;
			}
		}
		else
		{
			LOG(WARNING) << Image_Name_s << " Not Found At " << Images_path.make_preferred().string();
		}

		return true;
	}

	int count_files_in_directory(const std::filesystem::path& directory_path)
	{
		int count = 0;
		for (const auto& entry : std::filesystem::directory_iterator(directory_path))
		{
			if (std::filesystem::is_regular_file(entry))
			{
				++count;
			}
		}
		return count;
	}
	bool renderer::LoadTextureFrom_Gif_Files_Folder(std::string& Imagegif_Folder_Name, std::vector<std::pair<ID3D11ShaderResourceView*, std::string>>& gif_W_textures, int* gif_textures_number_total)
	{
		std::string directory_path = Imagegif_Folder_Name;

		//	std::string Imagegif_Name_s = Imagegif_Folder_Name + "/" + std::to_string(i) + ".png";
		auto Images_path            = std::filesystem::path(menu_DLL_Path);

		//	std::string Imagegif_Name_s = Imagegif_Folder_Name + "/" + std::to_string(i) + ".png";
		//	auto Images_path            = std::filesystem::path(menu_DLL_Path);
		Images_path /= directory_path;
		//LOG(INFO) << "Images directory path : " << Images_path.string().c_str() << std::endl;
		int nb_images_total = count_files_in_directory(Images_path);
		gif_W_textures.resize(nb_images_total);
		int nb_images = 0;
		for (const auto& entry : std::filesystem::directory_iterator(Images_path))
		{
			if (std::filesystem::is_regular_file(entry))
			{				
				//LOG(INFO) << "Nom du fichier : " << entry.path().filename().string().c_str();					
				//gif_W_textures.resize(nb_images);

				std::string Imagegif_Name_s = entry.path().string();
				std::string Images_path_s = Get_Patch_WFormat(Imagegif_Name_s);
				//LOG(INFO) << "Test to Load " << Imagegif_Name_s << " at : " << Images_path_s;

				// C'est sert a recuperer La taille de l'image Actuel Si besoin
				int Original_gif_width  = 0;
				int Original_gif_height = 0;

				//LOG(INFO) << "Images directory path : " << Images_path_s.c_str();

				bool ret = LoadTextureFromFile(Images_path_s.c_str(), &gif_W_textures[nb_images].first, &Original_gif_width, &Original_gif_height);
				if (!ret)
				{
					//LOG(FATAL) << "LoadTextureFromFile Failed do Load: " << Imagegif_Name_s;
					return false;
				}
				else
				{
					//// Trouver la position du premier point
					//size_t pos = entry.path().filename().string().find('.');		
					size_t pos                  = entry.path().filename().string().find('.');
					std::string chaine_modifiee = entry.path().filename().string().substr(0, pos);
					LOG(INFO) << "chaine_modifiee : " << chaine_modifiee;
					gif_W_textures[nb_images].second = chaine_modifiee;

					IM_ASSERT(ret);
				}	

				nb_images += 1;	
			}
		}
		*gif_textures_number_total = nb_images;
		//LOG(INFO) << "gif_W_textures_number_total: " << *gif_textures_number_total;
		return true;
	}
}
