#pragma once

#include "NoCopyMove.h"
#include <opencv2/opencv.hpp>
#include <ctime>
#include <vector>

namespace eg::bc
{
	// TODO: Move this to json
	constexpr size_t k_max_center_points = 30;
	constexpr size_t k_max_frame_count = k_max_center_points;
	constexpr size_t k_max_history = 4;

	enum class BoxStatus
	{
		Tracking,
		Counted,		// Moved from bottom to middle; left to middle to top; right to middle to top
		Rejected,		// Moved from bottom to middle to either left or right
		Returned,		// Moved from top to middle to either left or right
		Discarded,		// The should-be status after the box has been counted, rejected, or returned
		Unknown			// Does not fit any known status
	};

	enum BoxPolicyArea : size_t
	{
		Bottom = 0,
		Middle = 1,
		Top = 2,
		Left = 3,
		Right = 4,
		Unknown
	};

	class BoxPolicy :
		private eg::sys::NoCopyMove
	{
	public:
		BoxPolicy(
			const cv::Rect& bottom,
			const cv::Rect& middle,
			const cv::Rect& top,
			const cv::Rect& left,
			const cv::Rect& right) :
			rects_({ bottom, middle, top, left, right })
		{
		}

		BoxPolicyArea area_of(const cv::Point& center) const
		{
			if (rects_[BoxPolicyArea::Bottom].contains(center)) return BoxPolicyArea::Bottom;
			else if (rects_[BoxPolicyArea::Middle].contains(center)) 	return BoxPolicyArea::Middle;
			else if (rects_[BoxPolicyArea::Top].contains(center)) return BoxPolicyArea::Top;
			else if (rects_[BoxPolicyArea::Left].contains(center)) return BoxPolicyArea::Left;
			else if (rects_[BoxPolicyArea::Right].contains(center)) return BoxPolicyArea::Right;
			return BoxPolicyArea::Unknown;
		}

		const std::vector<cv::Rect>& rects() const
		{
			return rects_;
		}

	private:
		std::vector<cv::Rect> rects_;
	};

	class Box
	{
	public:

		explicit Box(const BoxPolicy& policy, const cv::Rect& rect) :
			policy_(policy),
			id_(gid_++),
			created_rect_(rect),
			updated_rect_(rect),
			created_at_(std::time(nullptr)),
			updated_at_(created_at_),
			frame_count_(1),
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
		}

		[[nodiscard]] size_t id() const
		{
			return id_;
		}

		[[nodiscard]] time_t elapsed_time() const
		{
			return updated_at_ - created_at_;
		}

		BoxStatus update(const cv::Rect& rect)
		{
			updated_rect_ = rect;
			updated_at_ = std::time(nullptr);
			++frame_count_;

			cv::Point center
			(
				rect.x + rect.width / 2,
				rect.y + rect.height / 2
			);

			center_points_.push_back(center);
			return update_status_(center);
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

		[[nodiscard]] size_t frame_count() const
		{
			return frame_count_;
		}

		[[nodiscard]] BoxStatus update_status_(const cv::Point& center)
		{
			// If it's discarded, it is assumed that the dev already got the previous status and
			// did some actionable items about it

			if (status_ == BoxStatus::Discarded)
			{
				return status_;
			}

			if (status_ == BoxStatus::Counted or status_ == BoxStatus::Rejected or status_ == BoxStatus::Returned)
			{
				status_ = BoxStatus::Discarded;
				return status_;
			}

			// Prep with BoxStatus::Unknown status if
			// it's the first time

			if (area_history_.empty())
			{
				area_history_.push_back(BoxPolicyArea::Unknown);
			}

			auto area = policy_.area_of(center);
			auto last_area = area_history_.back();

			if (last_area == BoxPolicyArea::Unknown and
				area not_eq BoxPolicyArea::Unknown)
			{
				area_history_.pop_back();
				area_history_.push_back(area);
			}

			else if (area not_eq last_area)
			{
				area_history_.push_back(area);
			}

			// Determine status based on area history
			if (area_history_.size() == 2)
			{
				// Counted: Bottom to Middle;
				if (area_history_[0] == BoxPolicyArea::Bottom and
					area_history_[1] == BoxPolicyArea::Middle)
				{
					status_ = BoxStatus::Counted;
					return status_;
				}

				// Rejected:
				// Bottom to Left or
				// Bottom to Right

				if (area_history_[0] == BoxPolicyArea::Bottom and (
					area_history_[1] == BoxPolicyArea::Left or area_history_[1] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Counted;
					return status_;
				}

				// Return:
				// Top to Left or
				// Top to Right

				if (area_history_[0] == BoxPolicyArea::Top and (
					area_history_[1] == BoxPolicyArea::Left or area_history_[1] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Counted;
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
					status_ = BoxStatus::Rejected;
					return status_;
				}

				if (area_history_[1] == BoxPolicyArea::Bottom and
					area_history_[2] == BoxPolicyArea::Middle and
					(area_history_[0] == BoxPolicyArea::Left or area_history_[0] == BoxPolicyArea::Right))
				{
					status_ = BoxStatus::Rejected;
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

			if (frame_count_ >= k_max_frame_count)
			{
				status_ = BoxStatus::Unknown;
				return status_;
			}

			return status_;
		}

	private:
		static inline size_t gid_ = 0;

		const BoxPolicy& policy_;

		size_t id_;
		cv::Rect created_rect_;
		cv::Rect updated_rect_;

		time_t created_at_;
		time_t	 updated_at_;
		size_t frame_count_;

		BoxStatus status_;

		// Center points will be used to debugging / presentation of
		// track movement direction

		std::vector<cv::Point> center_points_;
		std::vector<BoxPolicyArea> area_history_;
	};
}