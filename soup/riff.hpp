#pragma once

NAMESPACE_SOUP
{
	struct RiffChunk
	{
		std::string name;
		bool is_list;
		uint32_t data_size;
		size_t data_offset;

		[[nodiscard]] bool isValid() const noexcept
		{
			return !name.empty();
		}

		[[nodiscard]] std::string readData(Reader& r) const
		{
			std::string data;
			r.str(data_size, data);
			if (data_size & 1)
			{
				r.skip(1);
			}
			return data;
		}

		[[nodiscard]] size_t getDataEnd() const noexcept
		{
			return data_size + (data_size & 1) + data_offset;
		}

		void skipData(Reader& r) const
		{
			r.seek(getDataEnd());
		}
	};

	struct RiffReader
	{
		Reader& r;

		RiffReader(Reader& r)
			: r(r)
		{
		}

		[[nodiscard]] RiffChunk readChunk() const
		{
			RiffChunk chunk;
			r.str(4, chunk.name);
			chunk.is_list = (chunk.name == "RIFF" || chunk.name == "LIST");
			r.u32le(chunk.data_size);
			if (chunk.is_list)
			{
				chunk.data_size -= 4;
				r.str(4, chunk.name);
			}
			chunk.data_offset = r.getPosition();
			return chunk;
		}

		RiffChunk seekChunk(const std::string& name) const
		{
			r.seek(12);
			while (r.hasMore())
			{
				auto ck = readChunk();
				if (ck.name == name)
				{
					return ck;
				}
				ck.skipData(r);
			}
			return {};
		}
	};

	/*** Example: Recursively printing RIFF chunks
	* static void printRiffList(RiffReader& rr, size_t end)
	* {
	*     while (rr.r.getPosition() != end)
	*     {
	*         auto ck = rr.readChunk();
	*         if (ck.is_list)
	*         {
	*             std::cout << ck.name << " begins\n";
	*             printRiffList(rr, ck.getDataEnd());
	*             std::cout << ck.name << " ends\n";
	*         }
	*         else
	*         {
	*             std::cout << ck.name << "\n";
	*             ck.skipData(rr.r);
	*         }
	*     }
	* }
	* 
	* RiffReader rr(...);
	* auto ck = rr.readChunk();
	* printRiffList(rr, ck.getDataEnd());
	*/
}
