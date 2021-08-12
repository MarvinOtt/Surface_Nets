#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

//#define clamp(v, min, max) (v < min ? min : (v > max ? max : v))

#define LOADING_RADIUS 10
#define LOADING_HEIGHT 5

#define size 48
#define sizep4 (size+4)
#define sizep5 (size+5)
#define sizep6 (size+6)
#define sizem2 (size*size)
#define sizep4m2 (sizep4*sizep4)
#define sizep5m2 (sizep5*sizep5)
#define sizep6m2 (sizep6*sizep6)
#define dsize (size+6)
#define dsize2 (dsize*dsize)
#define vsize (size+5)
#define vsize2 (vsize*vsize)

float strength = 0.85f;
#define freq 0.0025f
SimplexNoise* simplexnoise;
inline Vector3 getnormal(Vector3 p1, Vector3 p2, Vector3 p3)
{
	return Vector3::Cross(p2 - p1, p3 - p1);
}
inline float clamp(float v, float min, float max)
{
	return (v < min ? min : (v > max ? max : v));
}

inline float getvalueatpos(float x, float y, float z, short* DATA)
{
	unsigned int ux = (unsigned int)x;
	unsigned int uy = (unsigned int)y;
	unsigned int uz = (unsigned int)z;

	short value_xyz = DATA[ux * dsize2 + uy * dsize + uz];
	short value_xYz = DATA[ux * dsize2 + (uy + 1) * dsize + uz];
	short value_Xyz = DATA[(ux + 1) * dsize2 + uy * dsize + uz];
	short value_XYz = DATA[(ux + 1) * dsize2 + (uy + 1) * dsize + uz];

	short value_xyZ = DATA[ux * dsize2 + uy * dsize + uz + 1];
	short value_xYZ = DATA[ux * dsize2 + (uy + 1) * dsize + uz + 1];
	short value_XyZ = DATA[(ux + 1) * dsize2 + uy * dsize + uz + 1];
	short value_XYZ = DATA[(ux + 1) * dsize2 + (uy + 1) * dsize + uz + 1];

	float xratio = x - (float)ux;
	float yratio = y - (float)uy;
	float zratio = z - (float)uz;

	float OUT1 = 0, OUT2 = 0;
	OUT1 += value_xyz * (1 - xratio) * (1 - yratio);
	OUT1 += value_xYz * (1 - xratio) * (yratio);
	OUT1 += value_Xyz * (xratio) * (1 - yratio);
	OUT1 += value_XYz * (xratio) * (yratio);
	OUT1 *= (1 - zratio);

	OUT2 += value_xyZ * (1 - xratio) * (1 - yratio);
	OUT2 += value_xYZ * (1 - xratio) * (yratio);
	OUT2 += value_XyZ * (xratio) * (1 - yratio);
	OUT2 += value_XYZ * (xratio) * (yratio);
	OUT2 *= zratio;

	return (OUT1 + OUT2) * 0.125f;
}

void getnewvertexpos(int x, int y, int z, byte state, short* DATA, Vector3* VertexPos)
{
	Vector3 direction = Vector3(0, 0, 0);
	if ((state & (1 << 0)) > 0)
		direction += Vector3(1, 1, 1);
	if ((state & (1 << 1)) > 0)
		direction += Vector3(1, -1, 1);
	if ((state & (1 << 2)) > 0)
		direction += Vector3(-1, 1, 1);
	if ((state & (1 << 3)) > 0)
		direction += Vector3(-1, -1, 1);

	if ((state & (1 << 4)) > 0)
		direction += Vector3(1, 1, -1);
	if ((state & (1 << 5)) > 0)
		direction += Vector3(1, -1, -1);
	if ((state & (1 << 6)) > 0)
		direction += Vector3(-1, 1, -1);
	if ((state & (1 << 7)) > 0)
		direction += Vector3(-1, -1, -1);

	/*int bot = 0, top = 0, right = 0, left = 0, back = 0, front = 0;
	if ((state & (1 << 0)) > 0)
		{ right++; top++; back++;}
	if ((state & (1 << 1)) > 0)
		{ right++; bot++; back++;}
	if ((state & (1 << 2)) > 0)
		{ left++; top++; back++;}
	if ((state & (1 << 3)) > 0)
		{ left++; bot++; back++;}

	if ((state & (1 << 4)) > 0)
		{ right++; top++; front++;}
	if ((state & (1 << 5)) > 0)
		{ right++; bot++; front++;}
	if ((state & (1 << 6)) > 0)
		{ left++; top++; front++;}
	if ((state & (1 << 7)) > 0)
		{ left++; bot++; front++;}

	Vector3 averagepos = Vector3(0, 0, 0);
	int count = 0;
	if (left < 4 && left > 0)
		{ averagepos.X--; count++;}
	if (right < 4 && right > 0)
		{ averagepos.X++; count++;}
	if (bot < 4 && bot > 0)
		{ averagepos.Y--; count++;}
	if (top < 4 && top > 0)
		{ averagepos.Y++; count++;}
	if (front < 4 && front > 0)
		{ averagepos.Z--; count++;}
	if (back < 4 && back > 0)
		{ averagepos.Z++; count++;}
	averagepos = averagepos / (float)count;*/

	if (direction.X > -0.00001f && direction.X < 0.00001f && direction.Y > -0.00001f && direction.Y < 0.00001f && direction.Z > -0.00001f && direction.Z < 0.00001f)
	{
		VertexPos[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1] = Vector3(x - 0.5f, y - 0.5f, z - 0.5f);
		return;
	}
	Vector3 averagepos = Vector3(0, 0, 0);
	direction = Vector3::Normalize(direction);
	Vector3 currentpos = Vector3(0, 0, 0);
	float value1 = getvalueatpos(x - 0.5f - direction.X * strength, y - 0.5f - direction.Y * strength, z - 0.5f - direction.Z * strength, DATA);
	//float value2 = getvalueatpos(x - 0.5f + direction.X * strength, y - 0.5f + direction.Y * strength, z - 0.5f + direction.Z * strength, DATA);
	Vector3 pos1 = Vector3(x - 0.5f - direction.X * strength, y - 0.5f - direction.Y * strength, z - 0.5f - direction.Z * strength);
	Vector3 pos2 = Vector3(x - 0.5f + direction.X * strength, y - 0.5f + direction.Y * strength, z - 0.5f + direction.Z * strength);
	//currentpos = Vector3(x - 0.5f, y - 0.5f, z - 0.5f);
	//float str = strength;
	for (int i = 0; i < 8; ++i)
	{
		/*currentpos = (pos1 + pos2) * 0.5f;
		float currentvalue = getvalueatpos(currentpos.X, currentpos.Y, currentpos.Z, DATA);
		if (currentvalue < 0)
		{
			if (value1 >= 0)
			{
				pos2 = currentpos;
				value2 = currentvalue;
			}
			else
			{
				pos1 = currentpos;
				value1 = currentvalue;
			}
		}
		else
		{
			if (value1 < 0)
			{
				pos2 = currentpos;
				value2 = currentvalue;
			}
			else
			{
				pos1 = currentpos;
				value1 = currentvalue;
			}
		}*/



		currentpos = (pos1 + pos2) * 0.5f;
		float currentvalue = getvalueatpos(currentpos.X, currentpos.Y, currentpos.Z, DATA);
		float dif1 = currentvalue - value1;
		if (currentvalue < 0)
			dif1 = -dif1;
		if (dif1 > 0)
			pos2 = currentpos;
		else
		{
			value1 = currentvalue;
			pos1 = currentpos;
		}

		/*pos1 = Vector3(currentpos.X - direction.X * str, currentpos.Y - direction.Y * str, currentpos.Z - direction.Z * str);
		pos2 = Vector3(currentpos.X + direction.X * str, currentpos.Y + direction.Y * str, currentpos.Z + direction.Z * str);
		float curval = getvalueatpos(currentpos.X, currentpos.Y, currentpos.Z, DATA);
		if (curval < 0)
			curval = -curval;
		float val1 = getvalueatpos(pos1.X, pos1.Y, pos1.Z, DATA);
		if (val1 < 0)
			val1 = -val1;
		float val2 = getvalueatpos(pos2.X, pos2.Y, pos2.Z, DATA);
		if (val2 < 0)
			val2 = -val2;

		if (val1 < val2 && val1 < curval)
		{
			currentpos = pos1;
		}
		else if (val2 < val1 && val2 < curval)
		{
			currentpos = pos2;
		}

		str *= 0.7f;*/
	}



	/*else if ((direction.X < -0.0001f || direction.X > 0.0001f) && (direction.Z < -0.0001f || direction.Z > 0.0001f))
		finalpos += direction * 0.5f;
	else if ((direction.Y < -0.0001f || direction.Y > 0.0001f) && (direction.Z < -0.0001f || direction.Z > 0.0001f))
		finalpos += direction * 0.5f;*/
	VertexPos[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1] = currentpos;// -Vector3(2.5f, 2.5f, 2.5f);
}

void SmoothVertex(int x, int y, int z, byte state, Vector3* VertexPos, Vector3* VertexPos_old)
{
	int bot = 0, top = 0, right = 0, left = 0, back = 0, front = 0;
	if ((state & (1 << 0)) > 0)
	{
		right++; top++; back++;
	}
	if ((state & (1 << 1)) > 0)
	{
		right++; bot++; back++;
	}
	if ((state & (1 << 2)) > 0)
	{
		left++; top++; back++;
	}
	if ((state & (1 << 3)) > 0)
	{
		left++; bot++; back++;
	}

	if ((state & (1 << 4)) > 0)
	{
		right++; top++; front++;
	}
	if ((state & (1 << 5)) > 0)
	{
		right++; bot++; front++;
	}
	if ((state & (1 << 6)) > 0)
	{
		left++; top++; front++;
	}
	if ((state & (1 << 7)) > 0)
	{
		left++; bot++; front++;
	}

	Vector3 averagepos = Vector3(0, 0, 0);
	int count = 0;
	if (left < 4 && left > 0)
	{
		averagepos += VertexPos_old[(x - 2) * vsize2 + (y - 1) * vsize + z - 1]; count++;
	}
	if (right < 4 && right > 0)
	{
		averagepos += VertexPos_old[(x - 0) * vsize2 + (y - 1) * vsize + z - 1]; count++;
	}
	if (bot < 4 && bot > 0)
	{
		averagepos += VertexPos_old[(x - 1) * vsize2 + (y - 2) * vsize + z - 1]; count++;
	}
	if (top < 4 && top > 0)
	{
		averagepos += VertexPos_old[(x - 1) * vsize2 + (y - 0) * vsize + z - 1]; count++;
	}
	if (front < 4 && front > 0)
	{
		averagepos += VertexPos_old[(x - 1) * vsize2 + (y - 1) * vsize + z - 2]; count++;
	}
	if (back < 4 && back > 0)
	{
		averagepos += VertexPos_old[(x - 1) * vsize2 + (y - 1) * vsize + z - 0]; count++;
	}
	if (count == 0)
	{
		VertexPos[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1] = VertexPos_old[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1];
		return;
	}
	averagepos = averagepos / (float)count;

	VertexPos[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1] = VertexPos_old[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1] * 0.2f + averagepos * 0.8f;// +Vector3(0, 1.0f * count, 0);;
}

void DLL_EXPORT SetSimplexNoise(int seed)
{
	simplexnoise = new SimplexNoise(seed);
	//simplexnoise = &SN;
}
void DLL_EXPORT SetParameters(float str)
{
	strength = str;
}

int DLL_EXPORT Generate_DATA(short* DATA, int x, int y, int z, int3 currentcampos)
{
	long long sum = 0;
	long long abssum = 0;
	for (int x2 = 0; x2 < size; ++x2)
	{
		for (int z2 = 0; z2 < size; ++z2)
		{
			float value = 1.75f * simplexnoise->octavenoise3d_H1(((x - currentcampos.X - LOADING_RADIUS) * size + x2) * freq, 2, ((z - currentcampos.Z - LOADING_RADIUS) * size + z2) * freq);// , 4, 0.456f);
			for (int y2 = 0; y2 < size; ++y2)
			{
				short data = (short)clamp(1000000.0f * (value - ((y - 1 - currentcampos.Y - LOADING_RADIUS + 2) * size + y2) * 0.01f + 0.5f), -32767.0f, 32767.0f);
				DATA[x2 * sizem2 + y2 * size + z2] = data;
				sum += data;
				abssum += llabs(data);
				//DATA[x2 * size2 + y2 * size + z2] = (short)clamp(100000.0f * simplexnoise->octavenoise3d_H1(((x - currentcampos.X) * size + x2) * freq, ((y - currentcampos.Y) * size + y2) * freq, ((z - currentcampos.Z) * size + z2) * freq), -32767.0f, 32767.0f);
			}
		}
	}
	if (llabs(sum) == abssum)
	{
		if (sum < 0)
			return -32767;
		else
			return 32767;
	}
	return 0;
}
DLL_EXPORT unsigned char* Generate_Chunk(Vector3* VertexPos, short* DATA, byte* IsVertex, int* returnlength)
{
	byte* states = (byte*)malloc(size * size * size + (sizep5) * (sizep5) * (sizep5) * 12 + (sizep6) * (sizep6) * (sizep6));
	Vector3* VertexPos_base = (Vector3*)(states + (size * size * size));
	byte* vertex_states = states + (size * size * size + (sizep5) * (sizep5) * (sizep5) * 12);

	for (int x = 1; x < sizep5; ++x)
	{
		for (int z = 1; z < sizep5; ++z)
		{
			for (int y = 1; y < sizep5; ++y)
			{
				bool IsValidG = false, IsValidL = false;
				if (DATA[(x)*dsize2 + (y)*dsize + (z)] > 0 && (DATA[(x + 1) * dsize2 + (y)*dsize + (z)] <= 0 || DATA[(x - 1) * dsize2 + (y)*dsize + (z)] <= 0 || DATA[(x)*dsize2 + (y + 1) * dsize + (z)] <= 0 || DATA[(x)*dsize2 + (y - 1) * dsize + (z)] <= 0 || DATA[(x)*dsize2 + (y)*dsize + (z + 1)] <= 0 || DATA[(x)*dsize2 + (y)*dsize + (z - 1)] <= 0))
					IsValidG = true;
				else if (DATA[(x)*dsize2 + (y)*dsize + (z)] <= 0 && (DATA[(x + 1) * dsize2 + (y)*dsize + (z)] > 0 || DATA[(x - 1) * dsize2 + (y)*dsize + (z)] > 0 || DATA[(x)*dsize2 + (y + 1) * dsize + (z)] > 0 || DATA[(x)*dsize2 + (y - 1) * dsize + (z)] > 0 || DATA[(x)*dsize2 + (y)*dsize + (z + 1)] > 0 || DATA[(x)*dsize2 + (y)*dsize + (z - 1)] > 0))
					IsValidL = true;
				if (IsValidG || IsValidL)
				{
					if (IsValidG)
					{
						if (DATA[(x + 1) * dsize2 + (y)*dsize + (z)] <= 0 && DATA[(x - 1) * dsize2 + (y)*dsize + (z)] <= 0)
							DATA[(x)*dsize2 + (y)*dsize + (z)] = 0;
						if (DATA[(x)*dsize2 + (y + 1) * dsize + (z)] <= 0 && DATA[(x)*dsize2 + (y - 1) * dsize + (z)] <= 0)
							DATA[(x)*dsize2 + (y)*dsize + (z)] = 0;
						if (DATA[(x)*dsize2 + (y)*dsize + (z + 1)] <= 0 && DATA[(x)*dsize2 + (y)*dsize + (z - 1)] <= 0)
							DATA[(x)*dsize2 + (y)*dsize + (z)] = 0;

						/*int count = 0;
						if (DATA[(x + 1)* dsize2 + (y)* dsize + (z)] <= 0) count++;
						if (DATA[(x - 1)* dsize2 + (y)* dsize + (z)] <= 0) count++;
						if (DATA[(x)* dsize2 + (y + 1)* dsize + (z)] <= 0) count++;
						if (DATA[(x)* dsize2 + (y - 1)* dsize + (z)] <= 0) count++;
						if (DATA[(x)* dsize2 + (y)* dsize + (z + 1)] <= 0) count++;
						if (DATA[(x)* dsize2 + (y)* dsize + (z - 1)] <= 0) count++;
						if (count >= 5)
							DATA[(x)* dsize2 + (y)* dsize + (z)] = 0;*/
					}
					else
					{
						if (DATA[(x + 1) * dsize2 + (y)*dsize + (z)] > 0 && DATA[(x - 1) * dsize2 + (y)*dsize + (z)] > 0)
							DATA[(x)*dsize2 + (y)*dsize + (z)] = 1;
						if (DATA[(x)*dsize2 + (y + 1) * dsize + (z)] > 0 && DATA[(x)*dsize2 + (y - 1) * dsize + (z)] > 0)
							DATA[(x)*dsize2 + (y)*dsize + (z)] = 1;
						if (DATA[(x)*dsize2 + (y)*dsize + (z + 1)] > 0 && DATA[(x)*dsize2 + (y)*dsize + (z - 1)] > 0)
							DATA[(x)*dsize2 + (y)*dsize + (z)] = 1;


						/*int count = 0;
						if (DATA[(x + 1)* dsize2 + (y)* dsize + (z)] > 0) count++;
						if (DATA[(x - 1)* dsize2 + (y)* dsize + (z)] > 0) count++;
						if (DATA[(x)* dsize2 + (y + 1)* dsize + (z)] > 0) count++;
						if (DATA[(x)* dsize2 + (y - 1)* dsize + (z)] > 0) count++;
						if (DATA[(x)* dsize2 + (y)* dsize + (z + 1)] > 0) count++;
						if (DATA[(x)* dsize2 + (y)* dsize + (z - 1)] > 0) count++;
						if (count >= 5)
							DATA[(x)* dsize2 + (y)* dsize + (z)] = 1;*/
					}

					if (IsValidL && DATA[(x + 1) * dsize2 + (y)*dsize + (z + 1)] <= 0 && DATA[(x + 1) * dsize2 + (y)*dsize + (z)] > 0 && DATA[(x)*dsize2 + (y)*dsize + (z + 1)] > 0)
					{
						DATA[(x)*dsize2 + (y)*dsize + (z)] = 1; DATA[(x + 1) * dsize2 + (y)*dsize + (z + 1)] = 1;
					}
					if (IsValidL && DATA[(x + 1) * dsize2 + (y)*dsize + (z - 1)] <= 0 && DATA[(x + 1) * dsize2 + (y)*dsize + (z)] > 0 && DATA[(x)*dsize2 + (y)*dsize + (z - 1)] > 0)
					{
						DATA[(x)*dsize2 + (y)*dsize + (z)] = 1; DATA[(x + 1) * dsize2 + (y)*dsize + (z - 1)] = 1;
					}

					if (IsValidL && DATA[(x + 1) * dsize2 + (y + 1) * dsize + (z)] <= 0 && DATA[(x + 1) * dsize2 + (y)*dsize + (z)] > 0 && DATA[(x)*dsize2 + (y + 1) * dsize + (z)] > 0)
					{
						DATA[(x)*dsize2 + (y)*dsize + (z)] = 1; DATA[(x + 1) * dsize2 + (y + 1) * dsize + (z)] = 1;
					}
					if (IsValidL && DATA[(x + 1) * dsize2 + (y - 1) * dsize + (z)] <= 0 && DATA[(x + 1) * dsize2 + (y)*dsize + (z)] > 0 && DATA[(x)*dsize2 + (y - 1) * dsize + (z)] > 0)
					{
						DATA[(x)*dsize2 + (y)*dsize + (z)] = 1; DATA[(x + 1) * dsize2 + (y - 1) * dsize + (z)] = 1;
					}

					if (IsValidL && DATA[(x)*dsize2 + (y + 1) * dsize + (z + 1)] <= 0 && DATA[(x)*dsize2 + (y + 1) * dsize + (z)] > 0 && DATA[(x)*dsize2 + (y)*dsize + (z + 1)] > 0)
					{
						DATA[(x)*dsize2 + (y)*dsize + (z)] = 1; DATA[(x)*dsize2 + (y + 1) * dsize + (z + 1)] = 1;
					}
					if (IsValidL && DATA[(x)*dsize2 + (y + 1) * dsize + (z - 1)] <= 0 && DATA[(x)*dsize2 + (y + 1) * dsize + (z)] > 0 && DATA[(x)*dsize2 + (y)*dsize + (z - 1)] > 0)
					{
						DATA[(x)*dsize2 + (y)*dsize + (z)] = 1; DATA[(x)*dsize2 + (y + 1) * dsize + (z - 1)] = 1;
					}
				}
			}
		}
	}
	for (int x = 1; x < sizep6; ++x)
	{
		for (int z = 1; z < sizep6; ++z)
		{
			for (int y = 1; y < sizep6; ++y)
			{
				unsigned char state1 = (DATA[x * dsize2 + y * dsize + z] > 0);
				unsigned char state2 = (DATA[x * dsize2 + (y - 1) * dsize + z] > 0);
				unsigned char state3 = (DATA[(x - 1) * dsize2 + y * dsize + z] > 0);
				unsigned char state4 = (DATA[(x - 1) * dsize2 + (y - 1) * dsize + z] > 0);
				unsigned char state5 = (DATA[x * dsize2 + y * dsize + z - 1] > 0);
				unsigned char state6 = (DATA[x * dsize2 + (y - 1) * dsize + z - 1] > 0);
				unsigned char state7 = (DATA[(x - 1) * dsize2 + y * dsize + z - 1] > 0);
				unsigned char state8 = (DATA[(x - 1) * dsize2 + (y - 1) * dsize + z - 1] > 0);

				unsigned char states = (state1 << 0) | (state2 << 1) | (state3 << 2) | (state4 << 3) | (state5 << 4) | (state6 << 5) | (state7 << 6) | (state8 << 7);
				vertex_states[x * sizep6m2 + y * sizep6 + z] = states;

				if (states != 0 && states != 255)
				{
					IsVertex[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + (z - 2 + 1)] = states;
					getnewvertexpos(x, y, z, states, DATA, VertexPos_base);
				}
				else
					IsVertex[(x - 2 + 1) * vsize2 + (y - 2 + 1) * vsize + z - 2 + 1] = 0;

			}
		}
	}

	for (int x = 1; x < sizep6; ++x)
	{
		for (int z = 1; z < sizep6; ++z)
		{
			for (int y = 1; y < sizep6; ++y)
			{
				byte states = vertex_states[x * sizep6m2 + y * sizep6 + z];
				if (states != 0 && states != 255)
				{
					//IsVertex[(x - 2) * vsize2 + (y - 2) * vsize + (z - 2)] = states;
					SmoothVertex(x, y, z, states, VertexPos, VertexPos_base);
				}
			}
		}
	}

	int counter = 0;
	//unsigned char* states = (unsigned char*)malloc(size * size * size);
	for (int x = 3; x < size + 3; ++x)
	{
		for (int z = 3; z < size + 3; ++z)
		{
			for (int y = 3; y < size + 3; ++y)
			{
				if (DATA[x * dsize2 + y * dsize + z] > 0)
				{
					bool state1 = (DATA[x * dsize2 + (y + 1) * dsize + z] > 0);
					bool state2 = (DATA[x * dsize2 + (y - 1) * dsize + z] > 0);
					bool state3 = (DATA[(x + 1) * dsize2 + y * dsize + z] > 0);
					bool state4 = (DATA[(x - 1) * dsize2 + y * dsize + z] > 0);
					bool state5 = (DATA[x * dsize2 + y * dsize + (z + 1)] > 0);
					bool state6 = (DATA[x * dsize2 + y * dsize + (z - 1)] > 0);
					byte states2 = (state1 << 0) | (state2 << 1) | (state3 << 2) | (state4 << 3) | (state5 << 4) | (state6 << 5);
					states[(x - 3) * size * size + (y - 3) * size + z - 3] = states2;
					if (!state1)
						counter++;
					if (!state2)
						counter++;
					if (!state3)
						counter++;
					if (!state4)
						counter++;
					if (!state5)
						counter++;
					if (!state6)
						counter++;
				}
			}
		}
	}
	/*
	for (int x = -1; x < size + 1; ++x)
	{
		for (int z = -1; z < size + 1; ++z)
		{
			for (int y = -1; y < size + 1; ++y)
			{
				if (DATA[(x + 3) * dsize2 + (y + 3) * dsize + (z + 3)] > 0)
				{
					int count = 0;
					if (DATA[(x + 3) * dsize2 + (y + 3 + 1) * dsize + (z + 3)] <= 0)
						count++;
					if (DATA[(x + 3 + 1) * dsize2 + (y + 3) * dsize + (z + 3)] <= 0)
						count++;
					if (DATA[(x + 3) * dsize2 + (y + 3) * dsize + (z + 3 - 1)] <= 0)
						count++;

					if (count == 3)
					{
						VertexPos[(x + 2 + 1) * vsize2 + (y + 2 + 1) * vsize + z + 1 + 1] = VertexPos[(x + 2 + 1) * vsize2 + (y + 2 + 1) * vsize + z + 1 + 1] * 0.15f + ((VertexPos[(x + 1 + 1) * vsize2 + (y + 2 + 1) * vsize + z + 1 + 1] + VertexPos[(x + 2 + 1) * vsize2 + (y + 1 + 1) * vsize + z + 1 + 1] + VertexPos[(x + 2 + 1) * vsize2 + (y + 2 + 1) * vsize + z + 2 + 1]) / 3.0f) * 0.85f;
					}
				}
				if (DATA[(x + 3) * dsize2 + (y + 3) * dsize + (z + 3)] <= 0)
				{
					int count = 0;
					if (DATA[(x + 3) * dsize2 + (y + 3 - 1) * dsize + (z + 3)] > 0)
						count++;
					if (DATA[(x + 3 - 1) * dsize2 + (y + 3) * dsize + (z + 3)] > 0)
						count++;
					if (DATA[(x + 3) * dsize2 + (y + 3) * dsize + (z + 3 + 1)] > 0)
						count++;

					if (count == 3)
					{
						VertexPos[(x + 1 + 1) * vsize2 + (y + 1 + 1) * vsize + z + 2 + 1] = VertexPos[(x + 1 + 1) * vsize2 + (y + 1 + 1) * vsize + z + 2 + 1] * 0.15f + ((VertexPos[(x + 2 + 1) * vsize2 + (y + 1 + 1) * vsize + z + 2 + 1] + VertexPos[(x + 1 + 1) * vsize2 + (y + 2 + 1) * vsize + z + 2 + 1] + VertexPos[(x + 1 + 1) * vsize2 + (y + 1 + 1) * vsize + z + 1 + 1]) / 3.0f) * 0.85f;
					}
				}
			}
		}
	}*/


	//VertexPositionColorNormal_noTexCoo *vertexes = (VertexPositionColorNormal_noTexCoo*)malloc(counter * 6 * 28);
	* returnlength = counter * 6;

	return states;
}

/*void DLL_EXPORT ClearArrayofReferences(void* pointer, int length)
{
	memset(pointer, 0, length);
}*/

void DLL_EXPORT FreeChunkGenerationMemory(unsigned char* states)
{
	free(states);
}

void DLL_EXPORT Generate_Vertexes(VertexPositionColorNormal_noTexCoo* vertexes, unsigned char* states, short* DATA, Vector3* VertexPos)
{
	int count = -1;
	Color col(0, 0, 0, 0);
	Vector3 normal, p1, p2, p3;
	for (int x2 = 3; x2 < size + 3; ++x2)
	{
		for (int z2 = 3; z2 < size + 3; ++z2)
		{
			for (int y2 = 3; y2 < size + 3; ++y2)
			{
				if (DATA[x2 * dsize2 + y2 * dsize + z2] > 0)
				{
					unsigned char state = states[(x2 - 3) * sizem2 + (y2 - 3) * size + z2 - 3];
					int x = x2 - 1 + 1;
					int y = y2 - 1 + 1;
					int z = z2 - 1 + 1;
					if ((state & (1 << 0)) == 0)
					{
						p1 = VertexPos[(x - 1) * vsize2 + y * vsize + z - 1]; p2 = VertexPos[x * vsize2 + y * vsize + z]; p3 = VertexPos[x * vsize2 + y * vsize + z - 1];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);

						p2 = VertexPos[(x - 1) * vsize2 + y * vsize + z]; p3 = VertexPos[x * vsize2 + y * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);
					}
					if ((state & (1 << 1)) == 0)
					{
						p2 = VertexPos[(x - 1) * vsize2 + (y - 1) * vsize + z - 1]; p1 = VertexPos[x * vsize2 + (y - 1) * vsize + z]; p3 = VertexPos[x * vsize2 + (y - 1) * vsize + z - 1];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);

						p1 = VertexPos[(x - 1) * vsize2 + (y - 1) * vsize + z]; p3 = VertexPos[x * vsize2 + (y - 1) * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);
					}
					if ((state & (1 << 2)) == 0)
					{
						p1 = VertexPos[x * vsize2 + (y - 1) * vsize + z - 1]; p2 = VertexPos[x * vsize2 + y * vsize + z]; p3 = VertexPos[x * vsize2 + (y - 1) * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);

						p2 = VertexPos[x * vsize2 + y * vsize + z - 1]; p3 = VertexPos[x * vsize2 + y * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);
					}
					if ((state & (1 << 3)) == 0)
					{
						p2 = VertexPos[(x - 1) * vsize2 + (y - 1) * vsize + z - 1]; p1 = VertexPos[(x - 1) * vsize2 + y * vsize + z]; p3 = VertexPos[(x - 1) * vsize2 + (y - 1) * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);

						p1 = VertexPos[(x - 1) * vsize2 + y * vsize + z - 1]; p3 = VertexPos[(x - 1) * vsize2 + y * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);
					}
					if ((state & (1 << 4)) == 0)
					{
						p1 = VertexPos[x * vsize2 + (y - 1) * vsize + z]; p2 = VertexPos[(x - 1) * vsize2 + y * vsize + z]; p3 = VertexPos[(x - 1) * vsize2 + (y - 1) * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);

						p2 = VertexPos[x * vsize2 + y * vsize + z]; p3 = VertexPos[(x - 1) * vsize2 + y * vsize + z];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);
					}
					if ((state & (1 << 5)) == 0)
					{
						p2 = VertexPos[x * vsize2 + (y - 1) * vsize + z - 1]; p1 = VertexPos[(x - 1) * vsize2 + y * vsize + z - 1]; p3 = VertexPos[(x - 1) * vsize2 + (y - 1) * vsize + (z - 1)];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);

						p1 = VertexPos[x * vsize2 + y * vsize + z - 1]; p3 = VertexPos[(x - 1) * vsize2 + y * vsize + z - 1];
						normal = Vector3::Normalize(getnormal(p1, p2, p3)); col = Color(-normal);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p1, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p3, normal, col);
						vertexes[++count] = VertexPositionColorNormal_noTexCoo(p2, normal, col);
					}

				}
			}
		}
	}

	//free(states);
}





extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// attach to process
		// return FALSE to fail DLL load
		break;

	case DLL_PROCESS_DETACH:
		// detach from process
		break;

	case DLL_THREAD_ATTACH:
		// attach to thread
		break;

	case DLL_THREAD_DETACH:
		// detach from thread
		break;
	}
	return TRUE; // succesful
}