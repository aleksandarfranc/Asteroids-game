// Asteroids at the command prompt using quick and simple c++ with some Maths.

#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

#include "olcConsoleGameEngine.h"

class OneLoneCoder_Asteroids : public olcConsoleGameEngine
{
public:
	OneLoneCoder_Asteroids()
	{
		m_sAppName = L"Asteroids";
	}

private:
	struct sSpaceObject
	{
		float x;
		float y;
		float dx;
		float dy;
		int nSize;
		float angle;
	};

	vector<sSpaceObject> vecAsteroids;
	vector<sSpaceObject> vecBullets;

	vector<pair<float, float>> ShipModel;
	vector<pair<float, float>> AsteroidModel;

	sSpaceObject player;
	int nScore = 0;

	bool bDead = false;

protected:
	// Called by olcConsoleGameEngine
	// It is used to create static resources. We're not using it to define initial state of the game which ResetGame function does.
	virtual bool OnUserCreate()
	{
		//
		ShipModel =
		{
			{0.0f, -5.5f},
			{-2.5f, 2.5f},
			{2.5f, 2.5f}
		};

		int AsteroidVerts = 20;
		AsteroidModel.resize(AsteroidVerts);
		for (int i = 0; i < AsteroidVerts; i++)
		{
			float a = (float(i) / (float)AsteroidVerts) * 6.283185f;
			float radius = ((float)rand() / (float)RAND_MAX) * 0.4f + 0.8f;
			AsteroidModel[i].first = cosf(a) * radius;
			AsteroidModel[i].second = sinf(a) * radius;
		}

		ResetGame();

		return true;
	}

	void ResetGame() {
		vecAsteroids.clear();
		vecBullets.clear();

		vecAsteroids.push_back({ 40.0f, 40.0f, -8.0f, -14.0f, (int)16, 0.0f });
		vecAsteroids.push_back({ 120.0f, 40.0f, 7.0f, 14.0f, (int)16, 0.0f });

		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		bDead = false;
		nScore = 0;
	}


	// Called by olcConsoleGameEngine
	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (bDead)
			ResetGame();

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		// rotating ship
		if (m_keys[VK_LEFT].bHeld)
		{
			player.angle -= 5.0f * fElapsedTime;
			if (player.angle < -3.14159f)
				player.angle = 3.14159f;
		}

		if (m_keys[VK_RIGHT].bHeld)
		{
			player.angle += 5.0f * fElapsedTime;
			if (player.angle > 3.14159f)
				player.angle = -3.14159f;
		}

		// acceleration
		if (m_keys[VK_UP].bHeld)
		{
			player.dx += 20 * fElapsedTime * (sinf(player.angle));
			player.dy += 20 * fElapsedTime * (-cosf(player.angle));
		}

		// fire bullets
		if (m_keys[VK_SPACE].bPressed)
		{
			vecBullets.push_back({ player.x , player.y, 50 * sinf(player.angle), -50 * cosf(player.angle),0,0 });
		}

		// velocity changes position
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		// keep ship in gamespace
		GoAround(player.x, player.y, player.x, player.y);

		// ship collision
		for (auto& a : vecAsteroids)
		{
			float Dist = sqrt((player.x - a.x) * (player.x - a.x) + (player.y - a.y) * (player.y - a.y));
			if (Dist <= a.nSize)
				bDead = true;
		}

		//update and draw asteroid
		for (auto& a : vecAsteroids)
		{
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;

			a.angle += 0.5f * fElapsedTime;

			GoAround(a.x, a.y, a.x, a.y);

			DrawWireFrame(AsteroidModel, a.x, a.y, a.angle, a.nSize, FG_YELLOW);
		}

		// new asteroids after splashig them
		vector<sSpaceObject> newAsteroids;

		//update and draw bullets
		for (auto& b : vecBullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;

			for (auto& a: vecAsteroids)
			{
				if (IsPointInTheCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					nScore += 100;
					b.x = -1000;
					if (a.nSize > 4)
					{
						float a1 = ((float)rand() / (float) RAND_MAX) * 6.283185f;
						float a2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						newAsteroids.push_back({ a.x + 10*sinf(a1), a.y - 10 * cosf(a1), sinf(a1) * 23.32f, -23.32f * cosf(a1), (int)a.nSize >> 1, 0.0f});
						newAsteroids.push_back({ a.x + 10*sinf(a1), a.y - 10 * cosf(a2), sinf(a2) * 23.32f, -23.32f * cosf(a2), (int)a.nSize >> 1, 0.0f});
					}
					a.x = -1000;
				}
			}

			olcConsoleGameEngine::Draw(b.x, b.y);
		}

		// append new asteroids
		for (auto& a : newAsteroids)
			vecAsteroids.push_back(a);
		newAsteroids.clear();

		// remove bullets
		if (vecBullets.size() != 0)
		{
			auto i = remove_if(vecBullets.begin(), vecBullets.end(), 
				[&](sSpaceObject o) { return (o.x < 1 || o.x >= ScreenWidth() - 1 || o.y < 1 || o.y >= ScreenHeight() - 1 ); 
			});
			if (i != vecBullets.end())
				vecBullets.erase( i);
		}

		// remove asteroids
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), 
				[&](sSpaceObject o)
				{
					return (o.x < -100); 
				});

			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		// add more asteroids in the game after destroying all of them
		if (vecAsteroids.empty())
		{
			nScore += 500;

			float a1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
			float a2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;

			vecAsteroids.push_back(
				{
					player.x + 60.0f * sinf(player.angle - 3.14/2.0f),
					player.y + 40.0f * cosf(player.angle - 3.14 / 2.0f),
					23.23f * sinf(a1) ,
					-23.32f * cosf(a1),
					(int)16, 0.0f
				});

			vecAsteroids.push_back(
				{ 
					player.x + 60.0f * sinf(player.angle + 3.14 / 2.0f),
					player.y + 40.0f * cosf(player.angle + 3.14 / 2.0f),
					23.23f * sinf(a2),
					-23.32f * cosf(a2),
					(int)16, 0.0f 
				});

		}

		// draw ship
		DrawWireFrame(ShipModel, player.x, player.y, player.angle, 1.0f, FG_WHITE);

		// draw score
		DrawString(2, 2, L"SCORE:" + to_wstring(nScore));

		return true;
	}

	bool IsPointInTheCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((cx - x) * (cx - x) + (cy - y) * (cy - y)) < radius;
	}

	void DrawWireFrame(const vector<pair<float, float>>& model, float x, float y, float fAngle, float fScale = 1.0f, short c = (short)9608)
	{		
		int size = model.size();
		vector<pair<float, float>> Transformed;
		Transformed.resize(size);

		//rotate
		for (int i = 0; i < size; i++)
		{
			Transformed[i].first = model[i].first * cosf(fAngle) - model[i].second * sinf(fAngle);
			Transformed[i].second = model[i].first * sinf(fAngle) + model[i].second * cosf(fAngle);
		}

		// scale
		for (int i = 0; i < size; i++)
		{
			Transformed[i].first *= fScale;
			Transformed[i].second *= fScale;
		}

		// transtale
		for (int i = 0; i < size; i++)
		{
			Transformed[i].first += x;
			Transformed[i].second += y;
		}

		// draw at screen
		for (int i = 0; i < size; i++)
		{
			DrawLine(Transformed[i].first, Transformed[i].second, Transformed[(i + 1) % size].first, Transformed[(i + 1) % size].second,PIXEL_SOLID,c);
		}
	}

	virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		GoAround(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}

	void GoAround(float x, float y, float& ox, float& oy)
	{
		ox = x;
		oy = y;

		if (x < 0.0f)
			ox = (float)ScreenWidth() + x;
		if (x >= (float)ScreenWidth())
			ox = x - (float)ScreenWidth();

		if (y < 0.0f)
			oy = (float)ScreenHeight() + y;
		if (y >= (float)ScreenHeight())
			oy = y - (float)ScreenHeight();
	}
	
};


int main()
{
	OneLoneCoder_Asteroids game;
	game.ConstructConsole(160, 100, 6, 6);
	game.Start();
	return 0;
}
