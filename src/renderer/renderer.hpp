#pragma once
#include "common.hpp"
#include "font_mgr.hpp"

namespace big
{
	using dx_callback      = std::function<void()>;
	using wndproc_callback = std::function<void(HWND, UINT, WPARAM, LPARAM)>;

	class renderer final
	{
	public:
		explicit renderer();
		~renderer() = default;

		font_mgr& get_font_mgr()
		{
			return m_font_mgr;
		}

		bool init();
		void destroy();

		/**
		 * @brief Add a callback function to draw your ImGui content in
		 * 
		 * @param callback Function
		 * @param priority The higher the priority the value the later it gets drawn on top
		 * @return true 
		 * @return false 
		 */
		bool add_dx_callback(dx_callback callback, uint32_t priority);
		/**
		 * @brief Add a callback function on wndproc
		 * 
		 * @param callback Function
		 */
		void add_wndproc_callback(wndproc_callback callback);

		void on_present();

		void rescale(float rel_size);

		void pre_reset();
		void post_reset();

		void wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);		

		std::vector<std::pair<ID3D11ShaderResourceView*, std::string>> blip_P_textures;		
		int blip_P_textures_number_total;
		bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
		bool LoadTextureFromFile_S(std::string& Image_Name_s , ID3D11ShaderResourceView** ID3D11ShaderResource_View);
		bool LoadTextureFrom_Gif_Files_Folder(std::string& Imagegif_Folder_Name, std::vector<std::pair<ID3D11ShaderResourceView*, std::string>>& gif_W_textures, int* gif_textures_number_total);

	private:
		static void new_frame();
		static void end_frame();

	private:
		IDXGISwapChain* m_dxgi_swapchain;
		ID3D11Device* m_d3d_device;
		ID3D11DeviceContext* m_d3d_device_context;

		std::map<uint32_t, dx_callback> m_dx_callbacks;
		std::vector<wndproc_callback> m_wndproc_callbacks;

		font_mgr m_font_mgr;
	};

	inline auto g_renderer = renderer();
}
