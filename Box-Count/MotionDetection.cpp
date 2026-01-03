#include "MotionDetection.h"

namespace eg::bc
{
	void convert_to_grayscale(const cv::Mat& input, cv::Mat& gray)
	{
		cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
	}

	// 3. Blur
	void blur_frame(cv::Mat& gray)
	{
		cv::GaussianBlur
		(
			gray,
			gray,
			cv::Size(k_blur_size, k_blur_size),
			0
		);
	}

	// 4. Initialize background
	void init_background(const cv::Mat& gray, cv::Mat& background)
	{
		gray.convertTo(background, CV_32F);
	}

	// 5. Update background
	void update_background(const cv::Mat& gray, cv::Mat& background)
	{
		cv::accumulateWeighted(gray, background, k_bg_alpha);
	}

	// 6. Frame difference
	void compute_frame_difference
	(
		const cv::Mat& gray,
		const cv::Mat& background,
		cv::Mat& diff
	)
	{
		cv::Mat bg_u8;
		background.convertTo(bg_u8, CV_8U);
		cv::absdiff(bg_u8, gray, diff);
	}

	// 7. Shadow suppression (THIS is what you were missing)
	void suppress_shadows(cv::Mat& diff)
	{
		cv::threshold
		(
			diff,
			diff,
			k_shadow_delta,
			255,
			cv::THRESH_TOZERO
		);
	}

	// 8. Threshold motion
	void threshold_motion(const cv::Mat& diff, cv::Mat& thresh)
	{
		cv::threshold
		(
			diff,
			thresh,
			k_motion_threshold,
			255,
			cv::THRESH_BINARY
		);
	}

	// 9. Morph cleanup
	void clean_motion_mask(cv::Mat& thresh)
	{
		//cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);
		//cv::erode(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 1);

		// TODO: iterations must be configurable
		cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 5);
		cv::erode(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 1);
	}

	// 10. Find contours
	std::vector<std::vector<cv::Point>> find_motion_contours(const cv::Mat& thresh)
	{
		std::vector<std::vector<cv::Point>> contours_all;
		std::vector<std::vector<cv::Point>> contours;

		cv::findContours
		(
			thresh,
			contours_all,
			cv::RETR_EXTERNAL,
			cv::CHAIN_APPROX_SIMPLE
		);

		for (const auto& c : contours_all)
		{
			if (cv::contourArea(c) >= k_min_contour_area)
			{
				contours.push_back(c);
			}
		}

		return contours;
	}

	// 11. Draw motion
	void draw_motion
	(
		cv::Mat& frame,
		const std::vector<std::vector<cv::Point>>& contours
	)
	{
		for (const auto& c : contours)
		{
			cv::rectangle
			(
				frame,
				cv::boundingRect(c),
				cv::Scalar(0, 0, 255),
				2
			);
		}
	}

	std::vector<cv::Rect> contours_to_boxes
	(
		const std::vector<std::vector<cv::Point>>& contours
	)
	{
		std::vector<cv::Rect> boxes;

		for (const auto& c : contours)
		{
			boxes.push_back(cv::boundingRect(c));
		}

		return boxes;
	}

	bool boxes_are_close
	(
		const cv::Rect& a,
		const cv::Rect& b,
		int padding
	)
	{
		cv::Rect a_expanded
		(
			a.x - padding,
			a.y - padding,
			a.width + padding * 2,
			a.height + padding * 2
		);

		return (a_expanded & b).area() > 0;
	}

	std::vector<cv::Rect> merge_boxes
	(
		const std::vector<cv::Rect>& input_boxes
	)
	{
		std::vector<cv::Rect> boxes = input_boxes;
		bool merged = true;

		while (merged)
		{
			merged = false;

			for (size_t i = 0; i < boxes.size(); ++i)
			{
				for (size_t j = i + 1; j < boxes.size(); ++j)
				{
					if
						(
							(boxes[i] & boxes[j]).area() > 0 ||
							boxes_are_close(boxes[i], boxes[j], k_merge_distance)
							)
					{
						boxes[i] = boxes[i] | boxes[j]; // UNION
						boxes.erase(boxes.begin() + j);
						merged = true;
						break;
					}
				}

				if (merged)
				{
					break;
				}
			}
		}

		return boxes;
	}

	void draw_boxes(cv::Mat& frame, const std::vector<cv::Rect>& boxes)
	{
		for (const auto& box : boxes)
		{
			cv::rectangle
			(
				frame,
				box,
				cv::Scalar(0, 255, 0),
				2
			);
		}
	}
}