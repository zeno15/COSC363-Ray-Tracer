#include "Torus.h"

#include "quarticSolver.h"

Torus::Torus(float _A, float _B, Color _color)
{
	m_A = _A;
	m_B = _B;
	color = _color;
}

Torus::~Torus()
{
}


float Torus::intersect(Vector pos, Vector dir, float *_tmax /*= nullptr*/)
{
	transformRay(pos, dir);

	//~ (x * x + y * y + z * z + A * A - B * B) ^ 2 = 4 * A * A * (x * x + y * y) ^ 2

	float G = 4.0f * std::pow(m_A, 2.0f) * (std::pow(dir.x, 2.0f) + std::pow(dir.y, 2.0f));
	float H = 8.0f * std::pow(m_A, 2.0f) * (pos.x * dir.x + pos.y * dir.y);
	float I = 4.0f * std::pow(m_A, 2.0f) * (std::pow(pos.x, 2.0f) + std::pow(pos.y, 2.0f));
	float J = std::pow(dir.x, 2.0f) + std::pow(dir.y, 2.0f) + std::pow(dir.z, 2.0f);
	float K = 2.0f * (pos.x * dir.x + pos.y * dir.y + pos.z * dir.z);
	float L = std::pow(pos.x, 2.0f) + std::pow(pos.y, 2.0f) + std::pow(pos.z, 2.0f) + std::pow(m_A, 2.0f) - std::pow(m_B, 2.0f);

	//~ (J * t * t + K * t + L) ^ 2 = G * t * t + H * t + I

	//~ Quartic coefficients

	float t4 = J * J;
	float t3 = 2.0f * J * K;
	float t2 = 2.0f * J * L + K * K - G;
	float t1 = 2.0f * K * L - H;
	float t0 = L * L - I;

	t0 /= t4;
	t1 /= t4;
	t2 /= t4;
	t3 /= t4;
	t4 /= t4;

	double ce[4]= {static_cast<double>(t3), static_cast<double>(t2), static_cast<double>(t1), static_cast<double>(t0)};
	double roots[4];

	unsigned int counts = quarticSolver(ce,roots);

	double min = 10000000.0f;

	for (unsigned int i = 0; i < counts; i += 1)
	{
		if (roots[i] > 0.0f && min > roots[i])
		{
			min = roots[i];
		}
	}

	if (_tmax != nullptr)
	{
		*_tmax = -1.0f;
	}

	return static_cast<float>(min == 10000000.0f ? -1.0f : min);
}

Vector Torus::normal(Vector p)
{
	transformRay(p, Vector());

	Vector n;


	Vector q = Vector(p.x, p.y, 0.0f) * m_A / (sqrtf(p.x * p.x + p.y * p.y));

	n = p - q;

	n.normalise();

	transformNormal(n);

	return n;
}

Color Torus::getColorTex(Vector _vec)
{
	return color;
}
	
void Torus::scale(Vector _scaleFactors)
{
}