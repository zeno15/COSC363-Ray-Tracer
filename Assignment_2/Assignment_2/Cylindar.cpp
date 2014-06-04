#include "Cylindar.h"

Cylindar::Cylindar(Color _color)
{
	
	scaleFact = Vector(1.0f, 1.0f, 1.0f);

	scale(scaleFact);

	color = _color;
	m_RadiusX = 1.0f;
	m_RadiusZ = 1.0f;
	m_Height = 1.0f;
	m_Origin = Vector(0.0f, 0.0f, 0.0f);
}

Cylindar::~Cylindar()
{
}


float Cylindar::intersect(Vector pos, Vector dir, float *_tmax /*= nullptr*/)
{
	transformRay(pos, dir);

	float a = std::pow(dir.x, 2.0f) / std::pow(m_RadiusX, 2.0f) + std::pow(dir.z, 2.0f) / std::pow(m_RadiusZ, 2.0f);

	float b = 2.0f * (pos.x * dir.x / std::pow(m_RadiusX, 2.0f) + pos.z * dir.z / std::pow(m_RadiusZ, 2.0f));

	float c = std::pow(pos.x, 2.0f) / std::pow(m_RadiusX, 2.0f) + std::pow(pos.z, 2.0f) / std::pow(m_RadiusZ, 2.0f) - 1.0f;

	if (b * b  - 4.0f * a * c < 0.0f)
	{
		return -1.0f;
	}

	float t1 = (-b + sqrtf(b * b  - 4.0f * a * c)) / (2.0f * a);
	float t2 = (-b - sqrtf(b * b  - 4.0f * a * c)) / (2.0f * a);

	float t = t1 > 0.0f ? (t2 > 0.0f ? (t1 < t2 ? t1 : t2) : -1.0f) : -1.0f;



	if (pos.y + dir.y * t < m_Origin.y)
	{
		//~ Bottom cap
		if (_tmax != nullptr)
		{
			*_tmax = t;
		}

		float tother = t1 == t ? t2 : t1;

		if (pos.y + dir.y * tother > m_Origin.y &&
			pos.y + dir.y * tother < m_Origin.y + m_Height)
		{
			t = (m_Origin.y - pos.y ) / dir.y;
		}
	}
	else if (pos.y + dir.y * t > m_Origin.y + m_Height)
	{
		//~ Top cap
		if (_tmax != nullptr)
		{
			*_tmax = t;
		}

		float tother = t1 == t ? t2 : t1;

		if (pos.y + dir.y * tother > m_Origin.y &&
			pos.y + dir.y * tother < m_Origin.y + m_Height)
		{
			t = (m_Origin.y + m_Height - pos.y ) / dir.y;
		}
	}
	else
	{
		if (_tmax != nullptr)
		{
			if (t == -1.0f)
			{
				*_tmax = -1.0f;
			}
			else if (t == t1)
			{
				*_tmax = t2;
			}
			else if (t == t2)
			{
				*_tmax = t1;
			}
		}
	}
	

	return t;
}

Vector Cylindar::normal(Vector p)
{
	transformRay(p, Vector());

	Vector n;

	if (std::abs(p.y - m_Origin.y) < 0.01f)
	{
		n = Vector(0.0f, -1.0f, 0.0f);
	}
	else if (std::abs(p.y - (m_Origin.y + m_Height)) < 0.01f)
	{
		n = Vector(0.0f, 1.0f, 0.0f);
	}
	else
	{
		n = Vector(p.x - m_Origin.x, 0.0f, p.z - m_Origin.z);

		float length = sqrtf(n.x * n.x + n.z * n.z);

		n = n / length;
	}

	return n;
}

Color Cylindar::getColorTex(Vector _vec)
{
	return color;
}

void Cylindar::scale(Vector _scaleFactors)
{
	m_RadiusX *= _scaleFactors.x;
	m_Height *= _scaleFactors.y;
	m_RadiusZ *= _scaleFactors.z;
}