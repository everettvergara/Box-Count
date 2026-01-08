#include "WxBoxCountFrame.h"
#include "Constants.h"
#include "MotionDetection.h"
#include "ESpeakHelper.h"

#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <espeak-ng/speak_lib.h>

#ifdef _WIN32
#include <objbase.h>
#endif

#include <opencv2/opencv.hpp>
#include <format>
#include <unordered_set>

namespace eg::bc
{
	// TODO: debug mode must be auto for footages/file
	// - save unidentified status tracks;
	// - save thresholded image to debug folder or whatever folder
	// - object ID must start with 1 every new transaction;
	// - audit image must be the original size of the video;
	// - validate fields before starting the counting
	// - [] open debug windows change to save using original size
	// - clicking the the preview image brings you to folder of the saved images;
	// - show trans id in the title bar;
	// - create a rowmanager
	// * add TTS services
	// * add calibrate camera
	// - add field for warehouse / customer name
	// - fix preview flickering
	// - set live_preview_ctr_ = 0 every new trans
	// - debug mode must disable live_preview_ctr_
	// - failed to read from camera should ask if the user wants to retry
	// - add logging service
	// - add config struct
	// - small preview must include the ID of the box
	// - only once instance of box counter per machine can be opened

	WxBoxCountFrame::WxBoxCountFrame(wxWindow* parent) :
		BoxCountFrame(parent),
		cams_(Cam::load_cams()),
		trans_gid_(BoxCountTrans::load_gid()),
		live_preview_ctr_(0),

		// TODO: 128 must be configurable
		frame_queue_(128),
		rect_queue_(128),
		signal_stop_(true)
	{
		// TODO: Graceful exit if failed to load gid

		// TODO: trans must be init at constructor
		trans_.clear(trans_gid_);
		trans_gid_ += 1;
		BoxCountTrans::save_gid(trans_gid_);

		on_init_();
		on_new_wx_states_();
	}

	void WxBoxCountFrame::on_init_text_doc_type_()
	{
		// TODO: doc_types must come from a config or database in the future

		const std::vector<wxString> doc_types =
		{
			"Picking List",
			"Delivery Receipt",
			"Sales Invoice",
			"Packing List",
			"Other Document"
		};

		for (const auto& doc_type : doc_types)
		{
			choice_doc_type->Append(doc_type);
		}

		if (doc_types.empty())
		{
			choice_doc_type->Append("Other Document");
		}

		choice_doc_type->SetSelection(0);
		trans_.doc_type = choice_doc_type->GetStringSelection().ToStdString();
	}

	void WxBoxCountFrame::on_init_text_doc_()
	{
		text_doc_no->SetValue(trans_.doc_no);
		picker_doc_date->SetValue(wxDateTime(trans_.doc_date));
		text_start_time->SetValue("");
		text_end_time->SetValue("");
	}

	void WxBoxCountFrame::on_init_counts_()
	{
		text_box_count->SetValue(std::to_string(trans_.box_count.size()));
		text_reject_count->SetValue(std::to_string(trans_.reject_count.size()));
		text_return_count->SetValue(std::to_string(trans_.return_count.size()));

		static constexpr int width = 160;
		static constexpr int height = 100;
		wxBitmap black_bitmap(width, height, 32);

		{
			wxMemoryDC dc(black_bitmap);
			dc.SetBackground(*wxBLACK_BRUSH);
			dc.Clear();
			dc.SelectObject(wxNullBitmap);
		}

		bitmap_last_box_count->SetBitmap(black_bitmap);
		//bitmap_last_box_count->Refresh();

		bitmap_last_reject_count->SetBitmap(black_bitmap);
		//bitmap_last_reject_count->Refresh();

		bitmap_last_return_count->SetBitmap(black_bitmap);
		//bitmap_last_return_count->Refresh();
	}

	void WxBoxCountFrame::on_init_live_()
	{
		static constexpr int width = 320;
		static constexpr int height = 200;
		wxBitmap black_bitmap(width, height, 32);

		{
			wxMemoryDC dc(black_bitmap);
			dc.SetBackground(*wxBLACK_BRUSH);
			dc.Clear();
			dc.SelectObject(wxNullBitmap);
		}

		bitmap_preview->SetBitmap(black_bitmap);
		//bitmap_preview->Refresh();
	}

	void WxBoxCountFrame::on_init_icon_()
	{
		wxIcon icon;
		icon.LoadFile(k_icon_filename.string(), wxBITMAP_TYPE_PNG);
		SetIcon(icon);
	}

	bool WxBoxCountFrame::on_new_trans_()
	{
		////trans_.clear(trans_gid_);

		//trans_gid_++;

		//if (not BoxCountTrans::save_gid(trans_gid_))
		//{
		//	return false;
		//}

		return true;
	}

	void WxBoxCountFrame::on_init_box_policy_()
	{
		// Get String selection from choice_box_policy
		auto selection = choice_camera->GetSelection();

		if (selection == wxNOT_FOUND)
		{
			// TODO: Log ERROR
			return;
		}

		auto client_data = static_cast<WxCamData*>(choice_camera->GetClientObject(selection));
		const auto& cam = cams_.at(client_data->data());

		box_policy_.set_area(BoxPolicyArea::Bottom, cv::Rect(cam.bottom.x, cam.bottom.y, cam.bottom.width, cam.bottom.height));
		box_policy_.set_area(BoxPolicyArea::Middle, cv::Rect(cam.middle.x, cam.middle.y, cam.middle.width, cam.middle.height));
		box_policy_.set_area(BoxPolicyArea::Top, cv::Rect(cam.top.x, cam.top.y, cam.top.width, cam.top.height));
		box_policy_.set_area(BoxPolicyArea::Left, cv::Rect(cam.left.x, cam.left.y, cam.left.width, cam.left.height));
		box_policy_.set_area(BoxPolicyArea::Right, cv::Rect(cam.right.x, cam.right.y, cam.right.width, cam.right.height));
	}

	void WxBoxCountFrame::on_init_cam_list_()
	{
		for (const auto& [data, cam] : cams_)
		{
			choice_camera->Append(cam.name, new WxCamData(data));
		}

		choice_camera->SetSelection(0);
	}

	void WxBoxCountFrame::on_init_()
	{
		on_init_icon_();
		on_init_text_doc_type_();
		on_init_text_doc_();
		on_init_counts_();
		on_init_cam_list_();
		on_init_live_();
	}

	void WxBoxCountFrame::on_button_new_(wxCommandEvent&)
	{
		frame_queue_.clear();
		rect_queue_.clear();
		active_boxes_.clear();
		trans_.clear(trans_gid_);

		trans_gid_ += 1;
		BoxCountTrans::save_gid(trans_gid_);

		on_init_text_doc_();
		on_init_counts_();
		on_init_live_();

		on_new_wx_states_();
	}

	void WxBoxCountFrame::on_button_close_(wxCommandEvent&)
	{
		stop_threads_();
		Close();
	}

	void WxBoxCountFrame::start_threads_()
	{
		signal_stop_ = false;

		cam_thread_ = std::thread(&WxBoxCountFrame::cam_loop_, this);
		motion_thread_ = std::thread(&WxBoxCountFrame::motion_loop_, this);
		counting_thread_ = std::thread(&WxBoxCountFrame::counting_loop_, this);
		tts_thread_ = std::thread(&WxBoxCountFrame::tts_loop_, this);
	}

	void WxBoxCountFrame::stop_threads_()
	{
		signal_stop_ = true;
		motion_cv_.notify_one();
		counting_cv_.notify_one();

		if (cam_thread_.joinable())
		{
			cam_thread_.join();
		}

		if (motion_thread_.joinable())
		{
			motion_thread_.join();
		}

		if (counting_thread_.joinable())
		{
			counting_thread_.join();
		}

		tts_cv_.notify_one();
		if (tts_thread_.joinable())
		{
			tts_thread_.join();
		}
	}

	void WxBoxCountFrame::on_button_start_(wxCommandEvent&)
	{
		// TODO: Add validation here...

		// Set Policy
		on_init_box_policy_();

		// Set trans_ data
		trans_.doc_type = choice_doc_type->GetStringSelection().ToStdString();
		trans_.doc_no = text_doc_no->GetValue().ToStdString();
		trans_.doc_date = picker_doc_date->GetValue().GetTicks();
		trans_.cam_data = static_cast<WxCamData*>(choice_camera->GetClientObject(choice_camera->GetSelection()))->data();
		trans_.start_time = std::time(nullptr);
		wxDateTime now(trans_.start_time);
		text_start_time->SetValue(now.Format("%m/%d/%Y %H:%M:%S"));
		trans_.show_track_history = check_show_track_history->GetValue();
		trans_.show_roi = check_show_roi->GetValue();

		start_threads_();
		on_start_wx_states_();
	}

	void WxBoxCountFrame::on_start_wx_states_()
	{
		choice_doc_type->Enable(false);
		text_doc_no->Enable(false);
		picker_doc_date->Enable(false);
		choice_camera->Enable(false);

		check_show_track_history->Enable(false);
		check_show_roi->Enable(false);

		button_start->Enable(false);
		button_stop->Enable(true);
		button_new->Enable(false);
	}

	void WxBoxCountFrame::on_stop_wx_states_()
	{
		choice_doc_type->Enable(false);
		text_doc_no->Enable(false);
		picker_doc_date->Enable(false);
		choice_camera->Enable(false);

		check_show_track_history->Enable(false);
		check_show_roi->Enable(false);

		button_start->Enable(false);
		button_stop->Enable(false);
		button_new->Enable(true);
	}

	void WxBoxCountFrame::on_new_wx_states_()
	{
		choice_doc_type->Enable(true);
		text_doc_no->Enable(true);
		picker_doc_date->Enable(true);
		choice_camera->Enable(true);

		check_show_track_history->Enable(true);
		check_show_roi->Enable(true);

		button_start->Enable(true);
		button_stop->Enable(false);
		button_new->Enable(false);
	}

	void WxBoxCountFrame::on_button_stop_(wxCommandEvent&)
	{
		stop_threads_();
		on_stop_wx_states_();
	}

	wxBitmap WxBoxCountFrame::cv_mat_to_wx_bitmap_(const cv::Mat& input)
	{
		assert(not input.empty());
		assert(input.type() == CV_8UC3);

		cv::Mat rgb;
		cv::cvtColor(input, rgb, cv::COLOR_BGR2RGB);

		wxImage image(rgb.cols, rgb.rows);
		std::memcpy(image.GetData(), rgb.data, rgb.total() * 3);
		return wxBitmap(image);
	}

	void WxBoxCountFrame::cam_loop_()
	{
		auto cap = [this]()
			{
				const auto& cam = cams_.at(trans_.cam_data);
				const auto& cam_type = cam.type;
				if (cam_type == "usb")
				{
					// TODO: Make sure cam_data is convertible to int
					return cv::VideoCapture(std::stoi(trans_.cam_data));
				}

				if (cam_type == "file" or cam_type == "rtsp")
				{
					return cv::VideoCapture(trans_.cam_data);
				}

				return cv::VideoCapture{};
			}();

		if (not cap.isOpened())
		{
			CallAfter([]() {wxMessageBox("Failed to open camera.", "Error", wxOK | wxICON_ERROR); });
			return;
		}

		size_t err = 0;

		do
		{
			for (size_t i = 0; i < k_frame_capture_after and not signal_stop_; ++i)
			{
				if (not cap.grab())
				{
					if (++err >= k_frame_max_error)
					{
						CallAfter([]() {wxMessageBox("Failed to read from camera.", "Error", wxOK | wxICON_ERROR); });
						signal_stop_ = true;
						break;
					}
				}
			}

			if (signal_stop_)
			{
				break;
			}

			cv::Mat frame;

			if (not cap.read(frame))
			{
				if (++err >= k_frame_max_error)
				{
					CallAfter([]() {wxMessageBox("Failed to read from camera.", "Error", wxOK | wxICON_ERROR); });
					signal_stop_ = true;
					break;
				}

				continue;
			}

			if (frame.empty())
			{
				continue;
			}

			// TODO: 320x200 must come from cam config
			cv::resize(frame, frame, cv::Size(320, 200));
			// TODO: Flip must come from cam config
			//cv::flip(frame, frame, 1);

			{
				std::lock_guard lock(motion_mutex_);
				frame_queue_.push_back(std::move(frame));
			}

			motion_cv_.notify_one();

			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
		} while (not signal_stop_);

		cap.release();
	}

	void WxBoxCountFrame::motion_loop_()
	{
		bool back_init = false;
		cv::Mat background;

		auto ctr = 1;

		while (true)
		{
			std::unique_lock  lock(motion_mutex_);
			motion_cv_.wait(lock, [this]()
				{
					return signal_stop_ or not frame_queue_.empty();
				});

			if (signal_stop_)
			{
				break;
			}

			auto frame = frame_queue_.pop_front();
			lock.unlock();

			// Process frame
			// TODO: Detect only box colors;

			cv::Mat gray;
			convert_to_grayscale(frame, gray);
			blur_frame(gray);

			if (not back_init)
			{
				init_background(gray, background);
				back_init = true;
				continue;
			}

			update_background(gray, background);

			cv::Mat diff;
			compute_frame_difference(gray, background, diff);
			suppress_shadows(diff);

			cv::Mat thresh;
			threshold_motion(diff, thresh);
			clean_motion_mask(thresh);

			auto contours = find_motion_contours(thresh);
			auto box_contours = contours_to_boxes(contours);

			// TODO: Any box_contours inside left and right policy must be removed before merging

			std::vector<cv::Rect> cleaned_box_contours;
			const auto& policy_area = box_policy_.rects();

			for (const cv::Rect box : box_contours)
			{
				const cv::Rect left_area = policy_area.at(BoxPolicyArea::Left);
				const cv::Rect right_area = policy_area.at(BoxPolicyArea::Right);
				const auto box_tl = box.tl();
				const auto box_br = cv::Point{ box.br().x - 1, box.br().y - 1 };

				if (left_area.contains(box_tl) and left_area.contains(box_br))
				{
					continue;
				}

				if (right_area.contains(box_tl) and right_area.contains(box_br))
				{
					continue;
				}

				cleaned_box_contours.push_back(box);
			}

			auto merged_boxes = merge_boxes(cleaned_box_contours);

			// TODO: Add to Debug Mode

			//// draw roi to thresh
			//const auto& rects = this->box_policy_.rects();
			//for (const auto& [area, rect] : rects)
			//{
			//	cv::rectangle(thresh, rect, cv::Scalar(255), 1);
			//}

			//// draw box_countours to thresh
			//for (const auto& bc : merged_boxes)
			//{
			//	cv::rectangle(thresh, bc, cv::Scalar(255), 1);
			//}

			//cv::imwrite(std::format("out/debug/box_contours_{}.png", ctr++), thresh);
			//cv::imshow("Box Countours", thresh);
			//cv::waitKey(1);

			{
				std::lock_guard lock(counting_mutex_);
				rect_queue_.push_back(std::pair(std::move(frame), std::move(merged_boxes)));
			}

			counting_cv_.notify_one();
		}

		//cv::destroyWindow("Box Countours");
	}

	void WxBoxCountFrame::counting_loop_()
	{
		size_t loop_frame = 0;
		std::unordered_set<size_t> counted_box_ids;
		std::unordered_set<size_t> rejected_box_ids;
		std::unordered_set<size_t> returned_box_ids;

		while (true)
		{
			std::unique_lock  lock(counting_mutex_);
			counting_cv_.wait(lock, [this]()
				{
					return signal_stop_.load() or not rect_queue_.empty();
				});

			if (signal_stop_.load())
			{
				break;
			}

			auto [frame, rects] = rect_queue_.pop_front();
			lock.unlock();

			// Iterate rects;

			// TODO: Improve after POC to handle direction
			std::vector<cv::Rect> new_rects;
			for (const auto& rect : rects)
			{
				// Check nearest acceptable rect of box in active_boxes_
				auto min_distance = std::numeric_limits<double>::max();
				auto min_box_itr = active_boxes_.end();
				for (auto box_itr = active_boxes_.begin(); box_itr != active_boxes_.end(); ++box_itr)
				{
					auto rect_center = cv::Point
					(
						rect.x + rect.width / 2,
						rect.y + rect.height / 2
					);

					auto box_center = box_itr->last_center_point();

					auto distance = cv::norm(rect_center - box_center);
					if (distance < min_distance)
					{
						min_distance = distance;
						min_box_itr = box_itr;
					}
				}

				if (min_box_itr not_eq active_boxes_.end() and min_distance <= k_max_acceptable_distance_to_associate_box)
				{
					min_box_itr->update(rect, loop_frame);
				}
				else
				{
					// if there's no acceptable distance... it could be that
					// this new rect and the person is picking up from left or right side

					// so check if this new rect overlaps in any of the active boxes;
					// if it overlaps then merge it with the closest to the center;

					auto min_distance = std::numeric_limits<double>::max();
					auto min_box_itr = active_boxes_.end();
					auto rect_center_point = cv::Point
					(
						rect.x + rect.width / 2,
						rect.y + rect.height / 2
					);

					for (auto box_itr = active_boxes_.begin(); box_itr != active_boxes_.end(); ++box_itr)
					{
						auto box_center_point = box_itr->last_center_point();

						if ((box_itr->updated_rect() & rect).area() > 0 and // If intersects
							cv::norm(rect_center_point - box_center_point) < min_distance)  // distance < min_distance
						{
							min_box_itr = box_itr;
						}
					}

					if (min_box_itr not_eq active_boxes_.end())
					{
						min_box_itr->update(rect, loop_frame);
					}
					else
					{
						new_rects.push_back(rect);
					}
				}
			}

			// Insert new boxes
			for (const auto& rect : new_rects)
			{
				active_boxes_.emplace_back(box_policy_, rect, loop_frame);
			}

			// Remove expired boxes and save if status is tracking or unknown;
			for (auto box_itr = active_boxes_.begin(); box_itr != active_boxes_.end(); )
			{
				if (box_itr->should_expire(loop_frame))
				{
					if (auto status = box_itr->status();
						status == BoxStatus::Tracking or status == BoxStatus::Unknown)
					{
						auto new_frame = frame.clone();

						if (trans_.show_roi)
						{
							const auto& rects = this->box_policy_.rects();
							for (const auto& [area, rect] : rects)
							{
								cv::rectangle(new_frame, rect, k_policy_area_color, 1);
							}
						}

						cv::rectangle(new_frame, box_itr->updated_rect(), k_box_tracking_color, 1);
						for (const auto& point : box_itr->center_points())
						{
							cv::circle(new_frame, point, 2, k_box_tracking_path_color, -1);
						}

						for (size_t i = 1; i < box_itr->center_points().size(); ++i)
						{
							cv::line(new_frame, box_itr->center_points()[i - 1], box_itr->center_points()[i], k_box_tracking_path_color, 1);
						}

						auto text = std::format("{}", box_itr->id());
						cv::putText(new_frame, text, cv::Point(box_itr->updated_rect().x, box_itr->updated_rect().y - 5),
							cv::FONT_HERSHEY_SIMPLEX, 0.4, k_text_tracking_color, 1);

						// Save new_frame to png
						std::filesystem::path out = std::format("out/trans/{}/box_{:05}_{}.png", trans_.id, box_itr->id(), box_itr->status_name());
						if (not std::filesystem::exists(out.parent_path()))
						{
							std::filesystem::create_directories(out.parent_path());
						}
						cv::imwrite(out.string(), new_frame);
					}
					box_itr = active_boxes_.erase(box_itr);
				}
				else
				{
					++box_itr;
				}
			}

			// Show active boxes
			cv::Mat last_frame_counted;
			cv::Mat last_frame_rejected;
			cv::Mat last_frame_returned;

			// Update counts
			for (const auto& box : active_boxes_)
			{
				const auto id = box.id();
				bool new_count = false;

				const auto status = box.status();

				if (status == BoxStatus::Counted)
				{
					if (counted_box_ids.find(id) == counted_box_ids.end())
					{
						counted_box_ids.insert(id);
						new_count = true;

						if (rejected_box_ids.contains(id)) rejected_box_ids.erase(id);
						if (returned_box_ids.contains(id)) returned_box_ids.erase(id);
					}
				}

				else if (status == BoxStatus::Rejected)
				{
					if (rejected_box_ids.find(id) == rejected_box_ids.end())
					{
						rejected_box_ids.insert(id);
						new_count = true;

						if (counted_box_ids.contains(id)) counted_box_ids.erase(id);
						if (returned_box_ids.contains(id)) returned_box_ids.erase(id);
					}
				}

				else if (status == BoxStatus::Returned)
				{
					if (returned_box_ids.find(id) == returned_box_ids.end())
					{
						returned_box_ids.insert(id);
						new_count = true;

						if (counted_box_ids.contains(id)) counted_box_ids.erase(id);
						if (rejected_box_ids.contains(id)) rejected_box_ids.erase(id);
					}
				}

				if (new_count)
				{
					// 1.0 Update last frame preview
					{
						auto box_color = k_box_tracking_color;
						auto box_path_color = k_box_tracking_path_color;
						auto text_color = k_text_tracking_color;
						auto status = box.status();
						if (status == BoxStatus::Counted)
						{
							box_color = k_box_counted_color;
							box_path_color = k_box_counted_path_color;
							text_color = k_text_counted_color;
						}
						else if (status == BoxStatus::Rejected)
						{
							box_color = k_box_rejected_color;
							box_path_color = k_box_rejected_path_color;

							text_color = k_text_rejected_color;
						}
						else if (status == BoxStatus::Returned)
						{
							box_color = k_box_returned_color;
							box_path_color = k_box_returned_path_color;
							text_color = k_text_returned_color;
						}

						cv::rectangle(frame, box.updated_rect(), box_color);

						// Draw track history
						for (const auto& point : box.center_points())
						{
							cv::circle(frame, point, 2, box_path_color, -1);
						}

						// Draw line of center_points
						for (size_t i = 1; i < box.center_points().size(); ++i)
						{
							cv::line(frame, box.center_points()[i - 1], box.center_points()[i], box_path_color, 1);
						}

						if (status == BoxStatus::Counted)
						{
							last_frame_counted = frame.clone();
							cv::resize(last_frame_counted, last_frame_counted, cv::Size(160, 100));
						}
						else if (status == BoxStatus::Rejected)
						{
							last_frame_rejected = frame.clone();
							cv::resize(last_frame_rejected, last_frame_rejected, cv::Size(160, 100));
						}
						else if (status == BoxStatus::Returned)
						{
							last_frame_returned = frame.clone();
							cv::resize(last_frame_returned, last_frame_returned, cv::Size(160, 100));
						}
					}
					// 2.0 Save to file for audit purposes:
					{
						const auto [box_color, box_path_color, text_color] =
							[&status]() -> std::tuple<cv::Scalar, cv::Scalar, cv::Scalar>
							{
								if (status == BoxStatus::Counted)
								{
									return { k_box_counted_color, k_box_counted_path_color, k_text_counted_color };
								}
								else if (status == BoxStatus::Rejected)
								{
									return { k_box_rejected_color, k_box_rejected_path_color, k_text_rejected_color };
								}
								else // if (status == BoxStatus::Returned)
								{
									return { k_box_returned_color, k_box_returned_path_color, k_text_returned_color };
								}
							}();

						auto new_frame = frame.clone();

						if (trans_.show_roi)
						{
							const auto& rects = this->box_policy_.rects();
							for (const auto& [area, rect] : rects)
							{
								cv::rectangle(new_frame, rect, k_policy_area_color, 1);
							}
						}

						cv::rectangle(new_frame, box.updated_rect(), box_color, 1);

						for (const auto& point : box.center_points())
						{
							cv::circle(new_frame, point, 2, box_path_color, -1);
						}

						for (size_t i = 1; i < box.center_points().size(); ++i)
						{
							cv::line(new_frame, box.center_points()[i - 1], box.center_points()[i], box_path_color, 1);
						}

						auto text = std::format("{}", box.id());
						cv::putText(new_frame, text, cv::Point(box.updated_rect().x, box.updated_rect().y - 5),
							cv::FONT_HERSHEY_SIMPLEX, 0.4, text_color, 1);

						// Save new_frame to png
						std::filesystem::path out = std::format("out/trans/{}/box_{:05}_{}.png", trans_.id, box.id(), box.status_name());
						if (not std::filesystem::exists(out.parent_path()))
						{
							std::filesystem::create_directories(out.parent_path());
						}
						cv::imwrite(out.string(), new_frame);
					}
				}
			}

			// TODO: optim, can be drawn outside CallAfter
			CallAfter([this,

				// Make a copy since it may expire before it is painted
				frame = frame.clone(),
				current_loop_frame = loop_frame,
				active_boxes = active_boxes_,

				last_frame_counted = std::move(last_frame_counted),
				last_frame_rejected = std::move(last_frame_rejected),
				last_frame_returned = std::move(last_frame_returned),

				counted_boxes_ = counted_box_ids.size(),
				rejected_boxes_ = rejected_box_ids.size(),
				returned_boxes_ = returned_box_ids.size()]()
			{
				if (trans_.show_roi)
				{
					const auto& rects = this->box_policy_.rects();
					for (const auto& [area, rect] : rects)
					{
						cv::rectangle(frame, rect, k_policy_area_color, 1);
					}
				}

				// Add text, show number of active boxes
				cv::putText(frame, std::format("Active: {}", active_boxes_.size()), cv::Point(10, 20),
					cv::FONT_HERSHEY_SIMPLEX, 0.4, k_text_color, 1);

				// Show ID and last_loop_frame of each box
				for (const auto& box : active_boxes)
				{
					auto box_color = k_box_tracking_color;
					auto box_path_color = k_box_tracking_path_color;
					auto text_color = k_text_tracking_color;

					if (box.status() == BoxStatus::Counted)
					{
						box_color = k_box_counted_color;
						box_path_color = k_box_counted_path_color;
						text_color = k_text_counted_color;
					}
					else if (box.status() == BoxStatus::Rejected)
					{
						box_color = k_box_rejected_color;
						box_path_color = k_box_rejected_path_color;
						text_color = k_text_rejected_color;
					}
					else if (box.status() == BoxStatus::Returned)
					{
						box_color = k_box_returned_color;
						box_path_color = k_box_returned_path_color;
						text_color = k_text_returned_color;
					}

					auto text = std::format("{}", box.id());
					cv::putText(frame, text, cv::Point(box.updated_rect().x, box.updated_rect().y - 5),
						cv::FONT_HERSHEY_SIMPLEX, 0.4, text_color, 1);

					cv::rectangle(frame, box.updated_rect(), box_color, 1);

					// Draw track history
					for (const auto& point : box.center_points())
					{
						cv::circle(frame, point, 2, box_path_color, -1);
					}

					// Draw line of center_points
					for (size_t i = 1; i < box.center_points().size(); ++i)
					{
						cv::line(frame, box.center_points()[i - 1], box.center_points()[i], box_path_color, 1);
					}
				}

				//
				if (live_preview_ctr_++ % 2 == 0)
				{
					auto bitmap = cv_mat_to_wx_bitmap_(frame);
					bitmap_preview->SetBitmap(bitmap);
				}

				if (not last_frame_counted.empty())
				{
					auto bitmap = cv_mat_to_wx_bitmap_(last_frame_counted);
					bitmap_last_box_count->SetBitmap(bitmap);

					// TODO: Make 1 parameterized in config
					if (counted_boxes_ % 1 == 0)
					{
						{
							std::lock_guard lock(tts_mutex_);
							queued_tts_.push(std::string("Box count is ") + std::to_string(counted_boxes_));
						}
						tts_cv_.notify_one();
					}
				}

				if (not last_frame_rejected.empty())
				{
					auto bitmap = cv_mat_to_wx_bitmap_(last_frame_rejected);
					bitmap_last_reject_count->SetBitmap(bitmap);
					{
						std::lock_guard lock(tts_mutex_);
						queued_tts_.push(std::string("Reject from conveyor is ") + std::to_string(rejected_boxes_));
					}
					tts_cv_.notify_one();
				}

				if (not last_frame_returned.empty())
				{
					auto bitmap = cv_mat_to_wx_bitmap_(last_frame_returned);
					bitmap_last_return_count->SetBitmap(bitmap);
					{
						std::lock_guard lock(tts_mutex_);
						queued_tts_.push(std::string("Reject from Truck is ") + std::to_string(returned_boxes_));
					}
					tts_cv_.notify_one();
				}

				// update counts
				text_box_count->SetValue(std::to_string(counted_boxes_));
				text_reject_count->SetValue(std::to_string(rejected_boxes_));
				text_return_count->SetValue(std::to_string(returned_boxes_));
			});

			loop_frame++;
		}
	}

	void WxBoxCountFrame::tts_loop_()
	{
#ifdef _WIN32
		if (HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED); FAILED(hr))
		{
			// TODO: Log Error
			return;
		}
#endif

		auto& espeak = EspeakNg::instance();
		if (not espeak.init())
		{
			// TODO: Log Error
			return;
		}

		ma_engine_config engine_config = ma_engine_config_init();
		engine_config.sampleRate = espeak.sample_rate;

		ma_engine ma_engine;

		if (ma_engine_init(&engine_config, &ma_engine) not_eq MA_SUCCESS)
		{
			// TODO: Log Error
			CoUninitialize();
			return;
		}

		while (true)
		{
			std::unique_lock lock(tts_mutex_);
			tts_cv_.wait(lock, [&, this]
				{
					return signal_stop_ or not queued_tts_.empty();
				});

			if (signal_stop_)
			{
				break;
			}

			auto front = std::move(queued_tts_.front());
			queued_tts_.pop();
			lock.unlock();

			const auto pcm_to_play = espeak.generate_pcm_from_text(front.c_str());

			ma_audio_buffer_config buffer_config =
				ma_audio_buffer_config_init(
					ma_format_s16,
					k_ma_channels,
					pcm_to_play.size() / k_ma_channels,
					pcm_to_play.data(),
					nullptr
				);

			ma_audio_buffer buffer{};
			if (ma_audio_buffer_init(&buffer_config, &buffer) not_eq MA_SUCCESS)
			{
				// TODO: Log ERROR here
				break;
			}

			ma_sound sound{};
			if (ma_sound_init_from_data_source(
				&ma_engine,
				&buffer,
				0,
				nullptr,
				&sound) not_eq MA_SUCCESS)
			{
				// TODO: Log Error here
				ma_audio_buffer_uninit(&buffer);
				ma_engine_uninit(&ma_engine);
				break;
			}

			ma_sound_start(&sound);

			while (ma_sound_is_playing(&sound)) std::this_thread::sleep_for(std::chrono::milliseconds(10));

			ma_sound_uninit(&sound);
			ma_audio_buffer_uninit(&buffer);
		}
		queued_tts_ = std::queue<std::string>{};
		ma_engine_uninit(&ma_engine);

#ifdef _WIN32
		CoUninitialize();
#endif
	}
}