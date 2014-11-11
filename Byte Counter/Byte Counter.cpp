// Byte Counter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <array>
#include <forward_list>
#include <iomanip>

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		std::cerr << "ERROR: Input file not specified" << std::endl;
		return 1;
	}

	size_t origsize = wcslen(argv[1]) + 6;
	char *newstr = new char[origsize];
	
	if (nullptr == newstr)
	{
		std::cerr << "ERROR: Failed allocate string buffer" << std::endl;
		return 1;
	}

	(void) memset(newstr, 0, origsize);

	size_t convertedChars = 0U;
	errno_t errn = wcstombs_s(&convertedChars, newstr, origsize, argv[1], _TRUNCATE);

	if (0 != errn)
	{
		std::cerr << "ERROR: Failed to read and convert file-name string" << std::endl;
		return 1;
	}
	
	std::ifstream ifs(newstr, std::ifstream::in | std::ifstream::binary);
	
	if (!ifs)
	{
		std::cerr << "ERROR: Failed to open file- " << newstr << std::endl;
		delete[] newstr;
		return 1;
	}

	std::array<int, 256> byte_stats = {};
	const size_t sizeofBuffer = 1024U;
	char * chbuffer = new char[sizeofBuffer];

	if (nullptr != chbuffer)
	{
		(void)memset(chbuffer, 0, sizeofBuffer);
		std::streamsize ii;
		int nn;

		while (ifs.good())
		{
			ifs.read(chbuffer, sizeofBuffer);

			for (ii = 0; ii < ifs.gcount(); ++ii)
			{
				nn = (int)(unsigned(chbuffer[ii]) & 0xff);
				byte_stats[nn]++;
			}
		}
	}

	ifs.close();

	long sum = 0L;

	for (int ii = 0; ii < 256; ++ii)
	{
		sum += byte_stats[ii];
	}

	typedef std::pair<int, int> BytePeriodPair;
	std::forward_list<BytePeriodPair> freq_list;

	for (int ii = 0; ii < 256; ++ii)
	{
		freq_list.push_front( std::make_pair(ii, sum / byte_stats[ii]) );
	}

	// Lambda (binary predicate) to sort list from most frequent to least frequent
	auto cmp = [](const BytePeriodPair& first, const BytePeriodPair& second) { return std::get<1>(first) < std::get<1>(second); };
	// Sort the list
	freq_list.sort(cmp);

	std::forward_list<BytePeriodPair>::const_iterator it = freq_list.cbegin();

	std::cout << std::get<0>(*it) << "," << std::get<1>(*it) << "," << sum << std::endl;
	it++;

	for (int ii=1; ii < 256; ++ii)
	{
		std::cout << std::get<0>(*it) << "," << std::get<1>(*it) << std::endl;
		it++;
	}

	(void) strcat_s(newstr, origsize, "(1)");
	std::ofstream ofs(newstr, std::ofstream::binary | std::ofstream::trunc | std::ofstream::out);

	if (!ofs)
	{
		std::cerr << "ERROR: Failed to open output file- " << newstr << std::endl;
		delete[] newstr;
		return 1;
	}

	bool fWrit = false;
	int ff = 0;

	if (nullptr != chbuffer)
	{
		for (long bb = 1; bb <= sum; ++bb)
		{
			it = freq_list.cbegin(); ++it;

			for (; it != freq_list.cend(); ++it)
			{
				if (0 == (bb % std::get<1>(*it)))
				{
					chbuffer[ff] = char(std::get<0>(*it));
					++ff;
					if (sizeofBuffer == ff)
					{
						ofs.write(chbuffer, sizeofBuffer);
						ff = 0;
					}
					fWrit = true;
				}
			}

			if (!fWrit)
			{
				chbuffer[ff] = char(0);
				++ff;
				if (sizeofBuffer == ff)
				{
					ofs.write(chbuffer, sizeofBuffer);
					ff = 0;
				}
			}

			fWrit = false;
		}

		ofs.write(chbuffer, ff);
		delete[] chbuffer;
	}

	delete[] newstr;
	ofs.close();

	return 0;
}
