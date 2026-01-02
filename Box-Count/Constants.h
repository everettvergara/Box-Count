#pragma once

#include <opencv2/opencv.hpp>

namespace eg::bc
{
	constexpr int k_frame_width = 640;
	constexpr int k_frame_height = 400;
	constexpr int k_frame_fps = 30;
	constexpr int k_frame_capture_after = 0; // 0 = noskip
	constexpr int k_frame_max_error = 10;
	const cv::Scalar k_policy_area_color = cv::Scalar(100, 100, 100);

	const cv::Scalar k_box_tracking_color = cv::Scalar(255, 0, 255);
	const cv::Scalar k_box_tracking_path_color = cv::Scalar(255, 0, 255);
	const cv::Scalar k_text_tracking_color = cv::Scalar(255, 0, 255);

	const cv::Scalar k_box_counted_color = cv::Scalar(0, 255, 0);
	const cv::Scalar k_box_counted_path_color = cv::Scalar(0, 255, 0);
	const cv::Scalar k_text_counted_color = cv::Scalar(0, 255, 0);

	const cv::Scalar k_box_returned_color = cv::Scalar(255, 0, 0);
	const cv::Scalar k_box_returned_path_color = cv::Scalar(255, 0, 0);
	const cv::Scalar k_text_returned_color = cv::Scalar(255, 0, 0);

	const cv::Scalar k_box_rejected_color = cv::Scalar(255, 0, 0);
	const cv::Scalar k_box_rejected_path_color = cv::Scalar(255, 0, 0);
	const cv::Scalar k_text_rejected_color = cv::Scalar(255, 0, 0);

	const cv::Scalar k_text_color = cv::Scalar(0, 255, 0);

	constexpr int k_max_acceptable_distance_to_associate_box = 50; // in a 320x200 pixels
}