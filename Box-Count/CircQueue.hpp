#pragma once

#include "NoCopyMove.h"			// eg::sys::NoCopyMove
#include <vector>				// std::vector
#include <functional>			// std::function
#include <cassert>				// assert
#include <mutex>				// std::mutex, std::lock_guard
#include <optional>

namespace eg::bc
{
	template<typename T>
	class CircQueue :
		private eg::sys::NoCopyMove
	{
	public:

		CircQueue(size_t c, std::function<void()> next) :
			data_(c + 1),
			wake_next_(std::move(next)),
			b_(0),	// Back write index
			f_(0)	// Front read index
		{
			assert(c >= 1);
			assert(wake_next_);
		}

		template<typename U>
			requires std::assignable_from<T&, U&&>
		void push_back(U&& frame)
		{
			{
				std::lock_guard l(mtx_);
				const auto c = data_.size();

				data_[b_] = std::forward<U>(frame);
				b_ = (b_ + 1) % c;

				// If it wraps around,
				// move the read index forward too;

				if (b_ == f_)
				{
					f_ = (f_ + 1) % c;
				}
			}

			// Wakeup consumers;
			if (wake_next_)
			{
				wake_next_();
			}
		}

		// pop_front() will be used by wake_process()
		// Moves the front frame out of the queue

		[[nodiscard]] std::optional<T> pop_front()
		{
			std::lock_guard l(mtx_);

			if (b_ == f_)
			{
				return {};
			}

			T frame = std::move(data_[f_]);
			f_ = (f_ + 1) % data_.size();

			return frame;
		}

		// peek_back() will be used by wake_preview()
		// Creates a copy of the last frame pushed

		[[nodiscard]] std::optional<T> peek_back() const
		{
			std::lock_guard l(mtx_);

			if (b_ == f_)
			{
				return {};
			}

			const auto c = data_.size();
			return data_[(b_ + c - 1) % c];
		}

		// Auxiliary methods

		[[nodiscard]] std::size_t size() const
		{
			std::lock_guard l(mtx_);

			if (b_ >= f_)
			{
				return b_ - f_;
			}

			return data_.size() - f_ + b_;
		}

		[[nodiscard]] bool empty() const
		{
			std::lock_guard l(mtx_);
			return b_ == f_;
		}

	private:

		mutable std::mutex mtx_;
		std::vector<T> data_;
		std::function<void()> wake_next_;

		// b_ is the index of the next frame to be written;
		// b_ always points to an invalid frame
		//		which will be overwritten on next push
		// b_ is the back of the queue,

		size_t b_;

		// f_ is the index of the next frame to be read;
		// f_ is the front of the queue

		size_t f_;
	};
}