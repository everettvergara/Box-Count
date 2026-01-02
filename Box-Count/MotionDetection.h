#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

namespace eg::bc
{
	constexpr int k_blur_size = 21;
	constexpr int k_motion_threshold = 25;
	constexpr int k_shadow_delta = 30;         // suppress low-intensity changes
	constexpr int k_min_contour_area = 100;
	constexpr double k_bg_alpha = 0.02;        // slow, stable background

	void convert_to_grayscale(const cv::Mat& input, cv::Mat& gray);
	void blur_frame(cv::Mat& gray);
	void init_background(const cv::Mat& gray, cv::Mat& background);
	void update_background(const cv::Mat& gray, cv::Mat& background);
	void compute_frame_difference(const cv::Mat& gray, const cv::Mat& background, cv::Mat& diff);
	void suppress_shadows(cv::Mat& diff);
	void threshold_motion(const cv::Mat& diff, cv::Mat& thresh);
	void clean_motion_mask(cv::Mat& thresh);
	std::vector<std::vector<cv::Point>> find_motion_contours(const cv::Mat& thresh);
	void draw_motion(cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours);
	std::vector<cv::Rect> contours_to_boxes(const std::vector<std::vector<cv::Point>>& contours);
	bool boxes_are_close(const cv::Rect& a, const cv::Rect& b, int padding);
	std::vector<cv::Rect> merge_boxes(const std::vector<cv::Rect>& input_boxes, int merge_distance);
	void draw_boxes(cv::Mat& frame, const std::vector<cv::Rect>& boxes);
}