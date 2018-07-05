#pragma once
#include <algorithm>

template <class InputContainer, class OutputContainer>
void doubleToFloat(const InputContainer& input, OutputContainer& output)
{
	std::transform(std::begin(input), std::end(input), std::begin(output), [](double n) {
		return static_cast<float>(n);
	});
}
