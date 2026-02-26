#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

namespace eg::bc
{
	//constexpr int k_blur_size = 21;
	//constexpr int k_motion_threshold = 25;
	//constexpr int k_shadow_delta = 30;         // suppress low-intensity changes
	//constexpr int k_min_contour_area = 100;
	//constexpr double k_bg_alpha = 0.02;        // slow, stable background

	// optimized for 320_x_200
	// TODO: must be configurable
	constexpr int k_blur_size = 21;
	constexpr int k_binarize_threshold = 25;	//
	constexpr int k_shadow_delta = 30;			// suppress low-intensity changes
	constexpr int k_min_contour_area = 50;
	constexpr double k_bg_alpha = 0.1;			// 0.0 - 1.0 (lower value leaves a trace) 0.02 = slow, stable background
	constexpr int k_merge_distance = 10;		// distance in px threshold to merge nearby boxes
	constexpr int k_dilate_iter = 13;			// defaults to 5
	constexpr int k_erode_iter = 1;				// TODO: must be configurable

	struct ComvisConfig
	{
		int blur_size = k_blur_size;
		int binarize_threshold = k_binarize_threshold;
		int shadow_delta = k_shadow_delta;
		int min_contour_area = k_min_contour_area;
		double bg_alpha = k_bg_alpha;
		int merge_distance = k_merge_distance;
		int dilate_iter = k_dilate_iter;
		int erode_iter = k_erode_iter;
		bool debug_contour = false;
	};

	void convert_to_grayscale(const cv::Mat& input, cv::Mat& gray);
	void blur_frame(const ComvisConfig& config, cv::Mat& gray);
	void init_background(const cv::Mat& gray, cv::Mat& background);
	void update_background(const ComvisConfig& config, const cv::Mat& gray, cv::Mat& background);
	void compute_frame_difference(const cv::Mat& gray, const cv::Mat& background, cv::Mat& diff);
	void suppress_shadows(const ComvisConfig& config, cv::Mat& diff);
	void binarize_color(const ComvisConfig& config, const cv::Mat& diff, cv::Mat& thresh);
	void clean_motion_mask(const ComvisConfig& config, cv::Mat& thresh);
	std::vector<std::vector<cv::Point>> find_motion_contours(const ComvisConfig& config, const cv::Mat& thresh);
	void draw_motion(cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours);
	std::vector<cv::Rect> contours_to_boxes(const std::vector<std::vector<cv::Point>>& contours);
	bool boxes_are_close(const cv::Rect& a, const cv::Rect& b, int padding);
	std::vector<cv::Rect> merge_boxes(const ComvisConfig& config, const std::vector<cv::Rect>& input_boxes);
	void draw_boxes(cv::Mat& frame, const std::vector<cv::Rect>& boxes);
}