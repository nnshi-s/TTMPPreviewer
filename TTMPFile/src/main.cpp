#include "Mdl.h"
#include "Model.h"
#include "Tex.h"
#include "DxtUtil.h"
#include "iomanip"
#include <chrono>

bool extractFile(const std::string &srcPath, const std::string &dstFileName, size_t start, size_t size)
{
	std::ifstream infile(srcPath + "TTMPD.mpd", std::ios::binary | std::ios::in);
	if (!infile.good())
	{
		std::cerr << "couldn't open" << std::endl;
		return false;
	}
	infile.seekg(start);
	if (!infile.good())
	{
		std::cerr << "couldn't seek" << std::endl;
		return false;
	}
	char *mem = new char[size];
	infile.read(mem, size);
	if (!infile.good())
	{
		std::cerr << "couldn't read" << std::endl;
		return false;
	}
	std::ofstream outfile(srcPath + dstFileName, std::ios::binary);
	//for (size_t i = 0; i < size; ++i)
	//	outfile << mem[i];
	outfile.write(mem, size);
	delete[] mem;
	std::cout << "extracted " + dstFileName << std::endl;
	return true;
}


int main()
{
	/*------ Researching mdl file ------*/
	// const std::string path = "G:/Projects/TTMPViewer/TTMPResearch/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/";
	// extractFile(path, "c0201e0204_top.mdl", 522880, 181248);

	//const std::string mdlPath = "G:/Projects/TTMPViewer/TTMPResearch/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/c0201e0204_top.mdl";
	//const std::wstring mpdPathDesktop = L"G:/Projects/TTMPViewer/TTMPResearch/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/TTMPD.mpd";
	//const std::wstring mpdPathNotebook = L"E:/Projects/TTMPViewer/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/TTMPD.mpd";
	//Tt::Mdl mdl;
	//std::ifstream ifs;
	//if (!mdl.readRawFromDisk(ifs, mpdPathDesktop, 522880, 181248))
	//	if (!mdl.readRawFromDisk(ifs, mpdPathNotebook, 522880, 181248))
	//		std::cerr << "couldn't read from file" << std::endl;
	//mdl.loadFromRaw();
	//// mdl.dumpDecompressedData();
	//mdl.print();
	//Tt::Model model;
	//model.loadFromMdl(mdl);

	/*------ Researching tex file ------*/
	//const std::string path = "G:/Projects/TTMPViewer/TTMPResearch/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/";
	//extractFile(path, "c0201e0204_top_d.tex", 704896, 4900864);
	//extractFile(path, "c0201e0204_top_n.tex", 5605760, 17926912);
	//extractFile(path, "c0201e0204_top_s.tex", 23532672, 20560896);
	// use _d.tex(diffuse map) as test.

	//const std::string texPath = "G:/Projects/TTMPViewer/TTMPResearch/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/c0201e0204_top_d.tex";
	//const std::wstring mpdPathDesktop = L"G:/Projects/TTMPViewer/TTMPResearch/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/TTMPD.mpd";
	//const std::wstring mpdPathNotebook = L"E:/Projects/TTMPViewer/TTMPFileResearch/Tight&Firm-FemmeFatale-SetB/TTMPD.mpd";
	//std::ifstream ifs;
	/*Tt::Tex tex;
	if (!tex.readRawFromDisk(ifs, mpdPathDesktop, 704896, 4900864))
		if (!tex.readRawFromDisk(ifs, mpdPathNotebook, 704896, 4900864))
			std::cerr << "couldn't read from file" << std::endl;
	tex.loadFromRaw();
	tex.print();
	auto st = std::chrono::system_clock::now();
	auto d = Tt::DxtUtil::decompressDxt1(tex.mpDecompressedDataRaw, tex.mWidth, tex.mHeight);
	auto et = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(et - st);
	std::cout << "decompressDxt1 time: " << duration.count() << "ms" << std::endl;
	
	return 0;*/
}
