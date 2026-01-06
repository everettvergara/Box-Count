#pragma once

#include "WxBoxCount.h"
#include "Data.hpp"
#include "Box.hpp"
#include "CircQueue.hpp"

#include <filesystem>
#include <unordered_map>
#include <string>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <tuple>

namespace eg::bc
{
	const std::filesystem::path k_icon_filename = "assets/images/icon.png";

	class WxCamData :
		public wxClientData
	{
	public:

		WxCamData(const std::string& cam_data) :
			cam_data_(cam_data)
		{
		}

		const std::string& data() const
		{
			return cam_data_;
		}
	private:
		std::string cam_data_;
	};

	class WxBoxCountFrame :
		public BoxCountFrame
	{
	public:
		WxBoxCountFrame(wxWindow* parent);

	private:

		// Master:
		// Loaded from configs

		const std::unordered_map<std::string, Cam> cams_;

		size_t trans_gid_;
		size_t live_preview_ctr_;

		// Trans
		BoxPolicy  box_policy_;
		BoxCountTrans trans_;
		CircQueue<cv::Mat> frame_queue_;
		CircQueue<std::pair<cv::Mat, std::vector<cv::Rect>>> rect_queue_;

		// TODO: Change to fixed sized vector
		// cannot be circ queue, since a newly detected box can be processed early

		// TODO: make it local to counting_loop_;
		std::list<Box> active_boxes_;

		// Threads
		std::atomic<bool> signal_stop_;

		std::thread cam_thread_;

		std::thread motion_thread_;
		std::condition_variable motion_cv_;
		std::mutex motion_mutex_;

		std::thread counting_thread_;
		std::condition_variable counting_cv_;
		std::mutex counting_mutex_;

		// TTS
		std::thread tts_thread_;
		std::condition_variable tts_cv_;
		std::mutex tts_mutex_;
		std::queue<std::string> queued_tts_;
		std::queue<std::vector<int16_t>> queued_pcm_;

		// Calibrate Thread
		std::thread calibrate_thread_;

		void on_init_();
		void on_init_icon_();
		void on_init_text_doc_type_();
		void on_init_text_doc_();
		void on_init_counts_();
		void on_init_live_();
		void on_init_cam_list_();
		void on_init_box_policy_();

		bool on_new_trans_();

		void on_button_preview_cam_(wxCommandEvent& event) override;
		void on_button_new_(wxCommandEvent& event) override;
		void on_button_close_(wxCommandEvent& event) override;
		void on_button_start_(wxCommandEvent& event) override;
		void on_button_stop_(wxCommandEvent& event) override;

		void on_start_wx_states_();
		void on_stop_wx_states_();
		void on_new_wx_states_();

		// Thread loopers
		void start_threads_();
		void stop_threads_();
		void cam_loop_();
		void motion_loop_();
		void counting_loop_();
		void tts_loop_();

		// TODO: Take out from the class
		wxBitmap cv_mat_to_wx_bitmap_(const cv::Mat& input);
	};
}