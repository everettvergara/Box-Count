#include "VideoCapture.h"

namespace eg::bc
{
	[[nodiscard]] bool VideoCapture::is_valid() const
	{
		return is_valid_;
	}

	[[nodiscard]] std::optional<cv::Mat> VideoCapture::read_frame()
	{
		// Do not perform any
		// operation if already invalid

		// Expecting to check is_valid()
		// before calling read_frame()

		assert(is_valid_);

		// Check if capture is opened,
		// if not, mark as invalid

		if (not is_opened())
		{
			is_valid_ = false;
			return std::nullopt;
		}

		auto update_retry_ctr =
			[&]()
			{
				if (++retry_ctr_ >= k_max_retries)
				{
					is_valid_ = false;
					capture_.reset();
				}
			};

		// Always grab the next frame
		// from the capture

		++n_;

		if (not capture_->grab())
		{
			update_retry_ctr();
			return std::nullopt;
		}

		retry_ctr_ = 0;

		if (n_ < every_n_frames_)
		{
			return std::nullopt;
		}

		n_ = 0;

		// Decode only the nth frame

		if (cv::Mat frame; capture_->retrieve(frame))
		{
			if (fx_ not_eq 1.0 or fy_ not_eq 1.0)
			{
				cv::resize(frame, frame, cv::Size(), fx_, fy_);
			}

			cv::flip(frame, frame, 1);

			return frame;
		}

		update_retry_ctr();
		return std::nullopt;
	}

	[[nodiscard]] bool VideoCapture::is_opened() const
	{
		return capture_.has_value() && capture_->isOpened();
	}
}