#pragma once

#include "NoCopyMove.h"			// eg::sys::NoCopyMove
#include <opencv2/opencv.hpp>	// cv::VideoCapture, cv::Mat
#include <optional>				// std::optional
#include <string>				// std::string
#include <cassert>				// assert
#include <concepts>				// std::same_as

namespace eg::bc
{
	constexpr size_t k_max_retries = 5;

	// Required for constructor and reconnect
	template<typename T>
	concept VideoCaptureSource =
		std::same_as<std::decay_t<T>, int> or
		std::same_as<std::decay_t<T>, std::string>;

	class VideoCapture
	{
	public:

		// Reads from a live camera
		// For production use

		template<VideoCaptureSource T>
		explicit VideoCapture(const T& source, size_t every_n_frames, double fx, double fy) :
			capture_(source),
			every_n_frames_(every_n_frames),
			fx_(fx), fy_(fy),
			n_(0),
			retry_ctr_(0),
			is_valid_(is_opened())
		{
			assert(fx_ > 0.0);
			assert(fy_ > 0.0);
			assert(every_n_frames_ >= 1);
		}

		template<VideoCaptureSource T>
		[[nodiscard]] bool reconnect(const T& source)
		{
			assert(not capture_.has_value());

			capture_.emplace(source);

			n_ = 0;
			retry_ctr_ = 0;

			is_valid_ = is_opened();
			return is_valid_;
		}

		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] bool is_opened() const;
		[[nodiscard]] std::optional<cv::Mat> read_frame();

	private:

		std::optional<cv::VideoCapture> capture_;

		size_t every_n_frames_;
		double fx_, fy_;
		size_t n_;
		size_t retry_ctr_;
		bool is_valid_;
	};
}