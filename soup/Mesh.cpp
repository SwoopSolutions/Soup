#include "Mesh.hpp"

#include "Poly.hpp"
#include "string.hpp"

NAMESPACE_SOUP
{
	Mesh Mesh::fromObj(const std::string& obj)
	{
		Mesh m;
		// Not ideal because std::stof or std::stoi may throw.
		SOUP_TRY
		{
			auto lines = string::explode<std::string>(obj, '\n');
			for (const auto& line : lines)
			{
				if (line.length() < 2)
				{
					continue;
				}
				if (line.substr(0, 2) == "v ")
				{
					auto ps = string::explode<std::string>(line.substr(2), ' ');
					if (ps.size() == 3)
					{
						m.verts.emplace_back(Vector3{ std::stof(ps[0]), std::stof(ps[1]), std::stof(ps[2]) });
					}
				}
				else if (line.substr(0, 2) == "f ")
				{
					auto is = string::explode<std::string>(line.substr(2), ' ');
					if (is.size() == 3)
					{
						m.tris.emplace_back(Tri{ (size_t)(std::stoi(is[0]) - 1), (size_t)(std::stoi(is[1]) - 1), (size_t)(std::stoi(is[2]) - 1) });
					}
				}
			}
		}
		SOUP_CATCH (std::exception, _)
		{
			m.clear();
		}
		return m;
	}

	bool Mesh::empty() const noexcept
	{
		return tris.empty();
	}

	void Mesh::clear() noexcept
	{
		verts.clear();
		tris.clear();
	}

	Vector3 Mesh::getCentrePoint() const
	{
		Vector3 centre{};
		for (const auto& p : verts)
		{
			centre += p;
		}
		centre /= (float)verts.size();
		return centre;
	}

	size_t Mesh::addVertex(const Vector3& p)
	{
		auto ret = verts.size();
		verts.emplace_back(p);
		return ret;
	}

	size_t Mesh::addVertex(float x, float y, float z)
	{
		return addVertex(Vector3{ x, y, z });
	}

	void Mesh::addTriangle(size_t a, size_t b, size_t c)
	{
		tris.emplace_back(Tri{ a, b, c });
	}

	void Mesh::addQuad(size_t a, size_t b, size_t c, size_t d)
	{
		addTriangle(c, b, a);
		addTriangle(d, c, a);
	}

	std::vector<Poly> Mesh::toPolys(const Vector3& transform) const
	{
		std::vector<Poly> polys{};
		for (const auto& tri : tris)
		{
			polys.emplace_back(Poly{
				transform + verts.at(tri.a),
				transform + verts.at(tri.b),
				transform + verts.at(tri.c)
				});
		}
		return polys;
	}
}
