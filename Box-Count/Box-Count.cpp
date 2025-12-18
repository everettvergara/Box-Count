#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

// -------------------- constants --------------------
constexpr int k_blur_size = 21;
constexpr int k_motion_threshold = 25;     // NOT 1, you animal
constexpr int k_shadow_delta = 30;         // suppress low-intensity changes
constexpr int k_min_contour_area = 100;
constexpr double k_bg_alpha = 0.02;        // slow, stable background

// -------------------- pipeline stages --------------------

// 1. Capture frame
bool capture_frame(cv::VideoCapture& cap, cv::Mat& frame, float scale = 1.0f)
{
	if (not cap.read(frame))
	{
		return false;
	}

	if (scale != 1.0f)
	{
		cv::resize
		(
			frame,
			frame,
			cv::Size
			(
				static_cast<int>(frame.cols * scale),
				static_cast<int>(frame.rows * scale)
			)
		);
	}

	cv::flip(frame, frame, 1);
	return true;
}

// 2. Convert to grayscale
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
	cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);
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
	const std::vector<cv::Rect>& input_boxes,
	int merge_distance
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
						boxes_are_close(boxes[i], boxes[j], merge_distance)
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

// -------------------- main --------------------
int main(int, char* [])
{
	try
	{
		cv::VideoCapture cap("assets/sample1.mp4");

		if (not cap.isOpened())
		{
			std::cerr << "Failed to open video\n";
			return EXIT_FAILURE;
		}

		cv::Mat background;
		bool background_initialized = false;

		while (true)
		{
			cv::Mat frame;
			if (not capture_frame(cap, frame, 0.5f))
			{
				break;
			}

			cv::Mat gray;
			convert_to_grayscale(frame, gray);
			blur_frame(gray);

			if (not background_initialized)
			{
				init_background(gray, background);
				background_initialized = true;
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
			auto merged_boxes = merge_boxes(box_contours, 200);
			draw_boxes(frame, merged_boxes);

			cv::imshow("motion", frame);
			cv::imshow("diff", diff);
			cv::imshow("mask", thresh);

			if (cv::waitKey(1) == 27)
			{
				break;
			}
		}

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}