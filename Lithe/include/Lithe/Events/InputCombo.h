#pragma once

#include "Input.h"

namespace Lithe {

	template<typename T>
	class InputCombo {
		enum class Operator { AND, OR };

		struct Evaluator {
			T input;
			State state;

			bool validate() const {
				if constexpr (std::is_same_v<T, Key>)
					return Input::isKey(input, state);
				else if constexpr (std::is_same_v<T, Button>)
					return Input::isMouse(input, state);

				return false;
			}
		};

		public:

			InputCombo& set(T input) {
				mInput = input;
				return *this;
			}

			InputCombo& is(State s) {
				Evaluator c(*mInput, s);
				mEvaluators.push_back(c);
				mInput.reset();
				mLastState = s;
				return *this;
			}

			InputCombo& and_() {
				finalizeCurrentCondition();

				mOperators.push_back(Operator::AND);
				mLastOperator = Operator::AND;
				return *this;
			}

			InputCombo& or_() {
				finalizeCurrentCondition();

				mOperators.push_back(Operator::OR);
				mLastOperator = Operator::OR;
				return *this;
			}

			operator bool() {
				finalizeCurrentCondition();
				return evaluateImpl(0, mEvaluators.size() - 1);
			}

			bool evaluateImpl(std::size_t left, std::size_t right) const {
				if (left > right)
					return false;
				else if (left == right)
					return mEvaluators[left].validate();

				const bool leftResult = mEvaluators[left].validate();
				const bool rightResult = evaluateImpl(left + 1, right);

				return (mOperators[left] == Operator::AND) ? leftResult && rightResult : leftResult || rightResult;
			}

		private:

			void finalizeCurrentCondition() {

				if (mInput) {
					Evaluator cond(*mInput, mLastState);
					mEvaluators.push_back(cond);
					mInput.reset();
				}

				if (!mEvaluators.empty() && mOperators.size() < mEvaluators.size() - 1)
					mOperators.push_back(mLastOperator);

			}

			std::optional<T> mInput;

			State mLastState = State::PRESSED;
			Operator mLastOperator = Operator::AND;

			std::vector<Evaluator> mEvaluators;
			std::vector<Operator> mOperators;

	};

}
