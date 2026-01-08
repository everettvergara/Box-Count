#pragma once

#include "NoCopyMove.h"
#include <opencv2/opencv.hpp>
#include <ctime>
#include <unordered_map>
#include <format>
#include <string>
#include <fstream>

namespace eg::bc
{
	// TODO: Move this to json
	constexpr size_t k_max_center_points = 64;
	constexpr size_t k_max_frame_count = k_max_center_points;
	constexpr size_t k_max_history = 4;
	constexpr size_t k_no_frame_update_expire = 5;

	enum class BoxStatus
	{
		Tracking,
		Counted,		// Moved from bottom to middle; left to middle to top; right to middle to top
		Rejected,		// Moved from bottom to middle to either left or right
		Returned,		// Moved from top to middle to either left or right
		Unknown			// Does not fit any known status
	};

	enum class BoxPolicyArea : size_t
	{
		Bottom,
		Middle,
		Top,
		Left,
		Right,
		Unknown
	};
}

namespace std
{
	template<>
	struct hash<eg::bc::BoxPolicyArea>
	{
		size_t operator()(const eg::bc::BoxPolicyArea v) const noexcept
		{
			return static_cast<size_t>(v);
		}
	};
}

namespace eg::bc
{
	//// TODO: Move this to json
	//constexpr size_t k_max_center_points = 30;
	//constexpr size_t k_max_frame_count = k_max_center_points;
	//constexpr size_t k_max_history = 4;

	//enum class BoxStatus
	//{
	//	Tracking,
	//	Counted,		// Moved from bottom to middle; left to middle to top; right to middle to top
	//	Rejected,		// Moved from bottom to middle to either left or right
	//	Returned,		// Moved from top to middle to either left or right
	//	Discarded,		// The should-be status after the box has been counted, rejected, or returned
	//	Unknown			// Does not fit any known status
	//};

	//enum class BoxPolicyArea : size_t
	//{
	//	Bottom,
	//	Middle,
	//	Top,
	//	Left,
	//	Right,
	//	Unknown
	//};

	class BoxPolicy :
		private eg::sys::NoCopyMove
	{
	public:

		BoxPolicy() :
			rects_({ { BoxPolicyArea::Bottom, cv::Rect{} }, { BoxPolicyArea::Middle, cv::Rect{} }, { BoxPolicyArea::Top, cv::Rect{} }, { BoxPolicyArea::Left, cv::Rect{} }, { BoxPolicyArea::Right, cv::Rect{} } })
		{
		}

		BoxPolicyArea area_of(const cv::Point& center) const
		{
			if (rects_.at(BoxPolicyArea::Bottom).contains(center)) return BoxPolicyArea::Bottom;
			else if (rects_.at(BoxPolicyArea::Middle).contains(center)) return BoxPolicyArea::Middle;
			else if (rects_.at(BoxPolicyArea::Top).contains(center)) return BoxPolicyArea::Top;
			else if (rects_.at(BoxPolicyArea::Left).contains(center)) return BoxPolicyArea::Left;
			else if (rects_.at(BoxPolicyArea::Right).contains(center)) return BoxPolicyArea::Right;
			return BoxPolicyArea::Unknown;
		}

		void set_area(BoxPolicyArea area, const cv::Rect& rect)
		{
			assert(area >= BoxPolicyArea::Bottom and area <= BoxPolicyArea::Right);
			rects_[area] = rect;
		}

		const std::unordered_map<BoxPolicyArea, cv::Rect>& rects() const
		{
			return rects_;
		}

	private:
		std::unordered_map<BoxPolicyArea, cv::Rect> rects_;
	};

	class Box
	{
	public:

		explicit Box(const BoxPolicy& policy, const cv::Rect& rect, size_t loop_frame) :
			policy_(policy),
			id_(gid_++),
			created_rect_(rect),
			updated_rect_(rect),
			created_at_(std::time(nullptr)),
			updated_at_(created_at_),
			loop_frame_(loop_frame),
			status_(BoxStatus::Tracking),
			center_points_(),
			area_history_()
		{
			cv::Point center
			(
				rect.x + rect.width / 2,
				rect.y + rect.height / 2
			);
			center_points_.reserve(k_max_center_points);
			area_history_.reserve(k_max_history);

			center_points_.push_back(center);
			status_ = update_status_(center, loop_frame);
		}

		[[nodiscard]] size_t id() const
		{
			return id_;
		}

		[[nodiscard]] time_t elapsed_time() const
		{
			return updated_at_ - created_at_;
		}

		void log(const std::string& text)
		{
			//if (id_ == 5)
			//{
			//	std::ofstream log_file("box_log.txt", std::ios::app);
			//	log_file << "[Box " << id_ << "] " << text << std::endl;
			//	//std::cout << "[Box " << id_ << "] " << text << std::endl;
			//}
		}

		BoxStatus update(const cv::Rect& rect, size_t current_loop_frame)
		{
			updated_rect_ = rect;
			updated_at_ = std::time(nullptr);
			loop_frame_ = current_loop_frame;

			cv::Point center
			(
				rect.x + rect.width / 2,
				rect.y + rect.height / 2
			);

			center_points_.push_back(center);
			return update_status_(center, current_loop_frame);
		}

		const std::vector<cv::Point>& center_points() const
		{
			return center_points_;
		}

		[[nodiscard]] const cv::Rect& created_rect() const
		{
			return created_rect_;
		}

		[[nodiscard]] const cv::Rect& updated_rect() const
		{
			return updated_rect_;
		}

		[[nodiscard]] size_t last_loop_frame() const
		{
			return loop_frame_;
		}

		[[nodiscard]] BoxStatus status() const
		{
			return status_;
		}

		[[nodiscard]] std::string status_name() const
		{
			switch (status_)

			{
			case BoxStatus::Counted:
				return "Counted";

			case BoxStatus::Rejected:
				return "Rejected";

			case BoxStatus::Returned:
				return "Returned";

			case BoxStatus::Tracking:
				return "Tracking";

			default:
				return "Unknown";
			}
		}

		[[nodiscard]] BoxStatus update_status_(const cv::Point& center, size_t current_loop_frame)
		{
			// If it's discarded, it is assumed that the dev already got the previous status and
			// did some actionable items about it
			//if (status_ == BoxStatus::Discarded)
			//{
			//	return status_;
			//}

			//if (status_ == BoxStatus::Counted or status_ == BoxStatus::Rejected or status_ == BoxStatus::Returned)
			//{
			//	//status_ = BoxStatus::Discarded;
			//	return status_;
			//}

			// Prep with BoxStatus::Unknown status if
			// it's the first time

			if (area_history_.empty())
			{
				log(std::format("0. area_history_ is empty, adding Unknown"));
				area_history_.push_back(BoxPolicyArea::Unknown);
			}

			log(std::format("1. center {}, {}", center.x, center.y));

			auto new_area = policy_.area_of(center);
			auto last_area = area_history_.back();

			if (last_area == BoxPolicyArea::Unknown and
				new_area not_eq BoxPolicyArea::Unknown)
			{
				area_history_.pop_back();
				area_history_.push_back(new_area);
				log(std::format("2. area_history_ {}", static_cast<int>(new_area)));
			}

			last_area = area_history_.back();
			log(std::format("3. last_area_ {}", static_cast<int>(last_area)));

			if (new_area not_eq last_area)
			{
				area_history_.push_back(new_area);
				log(std::format("4. last_area_ {} -> new area{}", static_cast<int>(last_area), static_cast<int>(new_area)));
			}

			// Determine status based on area history
			log(std::format("5. area_history_size_ {}", area_history_.size()));
			log("5x area_history_elements_:");
			for (size_t i = 0; i < area_history_.size(); ++i)
			{
				log(std::format("   - [{}] {}", i, static_cast<int>(area_history_[i])));
			}

			if (area_history_.size() == 2)
			{
				// Counted: Bottom to Middle;
				if (area_history_[0] == BoxPolicyArea::Bottom and
					area_history_[1] == BoxPolicyArea::Middle)
				{
					status_ = BoxStatus::Counted;
					return status_;
				}

				//// Counted: Bottom to Middle;
				//if (area_history_[0] == BoxPolicyArea::Middle and
				//	area_history_[1] == BoxPolicyArea::Top)
				//{
				//	status_ = BoxStatus::Counted;
				//	return status_;
				//}

				// Rejected:
				// Bottom to Left or
				// Bottom to Right

				if (area_history_[0] == BoxPolicyArea::Bottom and (
					area_history_[1] == BoxPolicyArea::Left or area_history_[1] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Rejected;
					return status_;
				}

				// Return:
				// Top to Left or
				// Top to Right

				if (area_history_[0] == BoxPolicyArea::Top and (
					area_history_[1] == BoxPolicyArea::Left or area_history_[1] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Returned;
					return status_;
				}

				if (area_history_[0] == BoxPolicyArea::Top and
					area_history_[1] == BoxPolicyArea::Middle)
				{
					status_ = BoxStatus::Returned;
					return status_;
				}
			}

			if (area_history_.size() == 3)
			{
				// Counted:
				// Left to Middle to Top
				// Right to Middle to Top
				// Left to Bottom to Middle
				// Right to Bottom to Middle

				if (area_history_[1] == BoxPolicyArea::Middle and
					area_history_[2] == BoxPolicyArea::Top and
					(area_history_[0] == BoxPolicyArea::Left or area_history_[0] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Counted;
					return status_;
				}

				if (area_history_[0] == BoxPolicyArea::Bottom and
					area_history_[1] == BoxPolicyArea::Middle and
					area_history_[2] == BoxPolicyArea::Top)
				{
					status_ = BoxStatus::Counted;
					return status_;
				}

				// Rejected:
				// Bottom to Middle to Left or
				// Bottom to Middle to Right

				if (area_history_[0] == BoxPolicyArea::Bottom and
					area_history_[1] == BoxPolicyArea::Middle and
					(area_history_[2] == BoxPolicyArea::Left or area_history_[2] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Rejected;
					return status_;
				}

				// Return:
				// Top to Middle to Left or
				// Top to Middle to Right

				if (area_history_[0] == BoxPolicyArea::Top and
					area_history_[1] == BoxPolicyArea::Middle and
					(area_history_[2] == BoxPolicyArea::Left or area_history_[2] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Returned;
					return status_;
				}
			}

			if (area_history_.size() == 4)
			{
				if (area_history_[0] == BoxPolicyArea::Bottom and
					area_history_[1] == BoxPolicyArea::Middle and
					area_history_[2] == BoxPolicyArea::Top and
					(area_history_[3] == BoxPolicyArea::Left or area_history_[3] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Rejected;
					return status_;
				}
			}

			if (should_expire(current_loop_frame))
			{
				status_ = BoxStatus::Unknown;
				return status_;
			}

			return status_;
		}

		cv::Point last_center_point() const
		{
			return center_points_.back();
		}

		bool should_expire(size_t current_loop_frame) const
		{
			return (current_loop_frame - loop_frame_) >= k_no_frame_update_expire;
			//return frame_count_ >= k_max_frame_count or status_ == BoxStatus::Discarded;
		}

	private:
		static inline size_t gid_ = 0;

		const BoxPolicy& policy_;

		size_t id_;
		cv::Rect created_rect_;
		cv::Rect updated_rect_;

		time_t created_at_;
		time_t	 updated_at_;
		size_t loop_frame_;

		BoxStatus status_;

		// Center points will be used to debugging / presentation of
		// track movement direction

		std::vector<cv::Point> center_points_;
		std::vector<BoxPolicyArea> area_history_;
	};
}