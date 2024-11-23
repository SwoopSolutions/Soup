#include "ZipWriter.hpp"

#include "crc32.hpp"
#include "Writer.hpp"
#include "ZipCentralDirectoryFile.hpp"
#include "ZipEndOfCentralDirectory.hpp"
#include "ZipLocalFileHeader.hpp"

NAMESPACE_SOUP
{
	ZipIndexedFile ZipWriter::addFile(std::string name, const std::string& contents_uncompressed, uint16_t compression_method, const std::string& contents_compressed) const
	{
		ZipIndexedFile zif;
		zif.compression_method = compression_method;
		zif.uncompressed_data_crc32 = crc32::hash(contents_uncompressed);
		zif.compressed_size = static_cast<uint32_t>(contents_compressed.size());
		zif.uncompressed_size = static_cast<uint32_t>(contents_uncompressed.size());
		zif.offset = static_cast<uint32_t>(os.getPosition());
		zif.name = std::move(name);

		ZipLocalFileHeader lfh{};
		lfh.common.compression_method = zif.compression_method;
		lfh.common.uncompressed_data_crc32 = zif.uncompressed_data_crc32;
		lfh.common.compressed_size = zif.compressed_size;
		lfh.common.uncompressed_size = zif.uncompressed_size;
		lfh.name = zif.name;
		os.raw(const_cast<char*>("\x50\x4b\x03\x04"), 4);
		lfh.write(os);

		os.raw(const_cast<char*>(contents_compressed.data()), contents_compressed.size());

		return zif;
	}

	ZipIndexedFile ZipWriter::addFileUncompressed(std::string name, const std::string& contents) const
	{
		return addFile(std::move(name), contents, 0, contents);
	}

	ZipIndexedFile ZipWriter::addFileAnticompressed(std::string name, const std::string& contents_uncompressed) const
	{
		std::string anti_compressed{};

		for (auto i = contents_uncompressed.cbegin(); i != contents_uncompressed.cend(); ++i)
		{
			anti_compressed.push_back(i == contents_uncompressed.cend() - 1);

			anti_compressed.push_back('\x1');
			anti_compressed.push_back('\x0');

			anti_compressed.push_back('\xFE');
			anti_compressed.push_back('\xFF');

			anti_compressed.push_back(*i);
		}

		return addFile(std::move(name), contents_uncompressed, 8, anti_compressed);
	}

	void ZipWriter::finalise(const std::vector<ZipIndexedFile>& files) const
	{
		ZipEndOfCentralDirectory eocd{};
		eocd.central_directories_on_this_disk = 1;
		eocd.central_directories_in_total = 1;
		eocd.central_directory_offset = static_cast<uint32_t>(os.getPosition());

		for (const auto& file : files)
		{
			ZipCentralDirectoryFile cdf{};
			cdf.common.compression_method = file.compression_method;
			cdf.common.uncompressed_data_crc32 = file.uncompressed_data_crc32;
			cdf.common.compressed_size = file.compressed_size;
			cdf.common.uncompressed_size = file.uncompressed_size;
			cdf.name = file.name;
			cdf.disk_offset = file.offset;
			//cdf.external_attributes = 2;
			os.raw(const_cast<char*>("\x50\x4b\x01\x02"), 4);
			cdf.write(os);
		}

		eocd.central_directory_size = ((uint32_t)os.getPosition() - eocd.central_directory_offset);
		os.raw(const_cast<char*>("\x50\x4b\x05\x06"), 4);
		eocd.write(os);
	}
}
