// This sample project demonstrates the performance issues arising when using std::unordered_map with
// hash values that share the same lower-order bits

#include <tchar.h>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <functional>
#include <numeric>
#include <random>
#include <boost/unordered_map.hpp>

// The key type holds several data items. 
struct Key
{
public:
	Key(std::uint32_t datum, std::string text)
		: Datum(datum),
		Text(text)
	{}

	bool operator==(const Key& rhs) const
	{
		return Datum == rhs.Datum && Text == rhs.Text;
	}

	std::uint32_t Datum;
	std::string Text;
};

// Hashers for Key
// In the original code where the issue occured, the key type contained two 32-bit unsigned integers, the combination 
// of which was guaranteed to be unique, so the hash value was generated by concatenating them, thus gauranteeing a 
// unique hash value per key.
// We have two hashers, which use the same 32-bit components as mentioned above, and differ only in the ordering - HasherA 
// concatenates Datum+Hash, while HasherB has the opposite order Hash+Datum
// As demonstrated by the code, these two seemingly equivalent hashers yield very different results for std::unordered_map
class HasherB
{
public:
	std::size_t operator()(Key key) const
	{
		auto textHash = std::hash<std::string>()(key.Text);
		return (static_cast<std::uint64_t>(key.Datum) << 32) | (textHash & 0xFFFFFFFF);
	}
};

class HasherA
{
public:
	std::size_t operator()(Key key) const
	{
		auto textHash = std::hash<std::string>()(key.Text);
		return textHash & 0xFFFFFFFF00000000 | key.Datum;
	}
};

template<typename MapType>
void PopulateMap(std::uint32_t numElements, MapType map)
{
	std::default_random_engine engine;
	auto distribution = std::uniform_int_distribution<std::uint32_t>(0, UINT32_MAX);

	std::string text = "all elements share the same text";
	for (std::uint32_t i = 0; i < numElements; ++i)
	{
		auto key = Key(distribution(engine), text);
		map[key] = i % 256;
	}
}

template<typename MapType>
std::chrono::duration<double, std::ratio<1,1>> RunTest(std::uint8_t numIterations, std::uint32_t numElements)
{
	std::vector<std::chrono::system_clock::duration> durations;
	MapType map;
	map.reserve(numElements);

	for (std::uint16_t i = 0; i < numIterations; ++i)
	{
		std::cout << "Iteration " << i << " started... ";
		auto startTime = std::chrono::system_clock::now();
		PopulateMap(numElements, map);
		map.clear();

		auto iteartionDuration = std::chrono::system_clock::now() - startTime;
		durations.emplace_back(iteartionDuration);
		
		std::cout << "ended. Duration: " 
			<< (double)(std::chrono::duration_cast<std::chrono::milliseconds>(iteartionDuration).count()) / 1000 
			<< std::endl;
	}

	auto InSeconds = std::accumulate(durations.begin(), durations.end(), std::chrono::duration<double, std::ratio<1,1>>(0));
	std::cout << "Total duration: " << InSeconds.count() << "; Average duration: " << InSeconds.count() / numIterations << std::endl;

	return InSeconds;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::uint8_t numIterations = 5;
	std::uint32_t numElements = 20000;

	auto boostDurationA = RunTest<boost::unordered_map<Key, std::uint8_t, HasherA>>(numIterations, numElements);
	auto stdDurationA = RunTest<std::unordered_map<Key, std::uint8_t, HasherA>>(numIterations, numElements);
	auto boostDurationB = RunTest<boost::unordered_map<Key, std::uint8_t, HasherB>>(numIterations, numElements);
	auto stdDurationB = RunTest<std::unordered_map<Key, std::uint8_t, HasherB>>(numIterations, numElements);
	
	std::cout << "Number of iterations " << static_cast<int>(numIterations) << std::endl;
	std::cout << "Number of elements handled per iteration " << numElements << std::endl;
	std::cout << "std::unordered_map runtime - HasherA: " << stdDurationA.count() << std::endl;
	std::cout << "boost::unordered_map runtime - HasherA: " << boostDurationA.count() << std::endl;
	std::cout << "Ratio (boost/std): " << boostDurationA.count() / stdDurationA.count() << std::endl;
	std::cout << "std::unordered_map runtime - HasherB: " << stdDurationB.count() << std::endl;
	std::cout << "boost::unordered_map runtime - HasherB: " << boostDurationB.count() << std::endl;
	std::cout << "Ratio (boost/std): " << boostDurationB.count() / stdDurationB.count() << std::endl;

	return 0;
}

