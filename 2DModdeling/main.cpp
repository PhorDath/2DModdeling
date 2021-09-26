/*
	this project is an attempt to create a 2d modelling tool

	Bellow is listed the references used in the project:
	pan and zoom: https://www.youtube.com/watch?v=ZQ8qtAizis4
	cad: https://www.youtube.com/watch?v=kxKKHKSMGIg
*/

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>
#include <PhysicsEngine2D.h>
#include <vector>
#include <string>

struct point {
	float x = 0.0f, y = 0.0f;

	point() {}
	point(float x, float y) {
		this->x = x;
		this->y = y;
	}
};

class edge {
public:
	point pStart, pEnd;

public:
	edge() {}
	edge(point start, point end) {
		this->pStart = start;
		this->pEnd = end;
	}
	void draw() {
	
	}
};

class polygon {
public:
	std::vector<point> vertices;

public:
	polygon() {}

	void draw() {

	}
};


class GUI : public olc::PixelGameEngine
{
private:
	polygon pTempPolygon;
	std::vector<edge> vEdges;
	std::vector<polygon> vPolygons; // store every polygon generated
	point pOffset;
	point pStartPan;
	float fScale = 20.0f;
	float fMaxScale = 8 * 64.0f, fMinScale = 10.0f;;
	float fGrid = 1.0f;

	bool bClipping = true; // determine if cursor will clip to the grid
	float fClippingRadius = 3.0f;

	bool bShowGrid = true;

	bool bCreatingPolygon = false;
	point pNextPoint;

	point pOffsetPoint;

	bool bShowCursor = true;

	bool bInstructions = false;

	void WorldToScreen(point pScreen, point &pView); // convert
	void ScreenToWorld(point pScreen, point &pView); // convert

	double DistancePointToPoint(point p1, point p2);
	double DistancePointToEdge(point p1, point p2, point p3);
	point LineIntersection(point p1, point p2, point p3, point p4);
	bool IsPointInLineSeg(point p1, point p2, point p3);
	point ClosestPointFromLine(point p1, point p2, point p3);

public:
	GUI();

	bool OnUserCreate() override;

	bool OnUserUpdate(float fElapsedTime) override;
};

int main()
{
	GUI demo;
	if (demo.Construct(1260, 720, 2, 2, false))
	//if (demo.Construct(3840, 2160, 1, 1, true))
	//if (demo.Construct(960, 540, 4, 4, false))
		demo.Start();
	return 0;
}

void GUI::WorldToScreen(point pWorld, point &pScreen) {
	pScreen.x = (int)((pWorld.x - pOffset.x) * fScale);
	pScreen.y = (int)((pWorld.y - pOffset.y) * fScale);
}

void GUI::ScreenToWorld(point pScreen, point &pWorld) {
	pWorld.x = pScreen.x / fScale + pOffset.x;
	pWorld.y = pScreen.y / fScale + pOffset.y;
}

double GUI::DistancePointToPoint(point p1, point p2){
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

double GUI::DistancePointToEdge(point p1, point p2, point p3) {
	float a, b, c;

	a = p2.y - p3.y;
	b = p3.x - p2.x;
	c = (p2.x * p3.y) - (p2.y * p3.x);

	return abs(a * p1.x + b * p1.y + c) / sqrt(a * a + b * b);
}

point GUI::LineIntersection(point p1, point p2, point p3, point p4) {
	// source: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	point p;
	float d = (p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x);
	
	p.x = (p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x);
	p.x /= d;

	p.y = (p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x);
	p.y /= d;

	return p;
}

bool GUI::IsPointInLineSeg(point p1, point p2, point p3) {
	if (DistancePointToPoint(p1, p2) + DistancePointToPoint(p1, p3) >= DistancePointToPoint(p2, p3) && 
		DistancePointToPoint(p1, p2) + DistancePointToPoint(p1, p3) <= DistancePointToPoint(p2, p3) + (fClippingRadius / fScale)) {
		return true;
	}
	return false;
}

point GUI::ClosestPointFromLine(point p1, point p2, point p3) {
	// find a perpendicular line with line (p2, p3) that pass through point p3

	/*
	// using the slope formula
	//source: https://www.youtube.com/watch?v=YbHOzJIHS1k
	// get values a, b and c from the line
	// the line equantion is given in the form y = mx - b
	float x, y, b;

	x = p2.y - p3.y;
	y = p3.x - p2.x;
	b = (p2.x * p3.y) - (p2.y * p3.x);

	// set y to 1
	x = x / y;
	b = b / y;
	y = y / y;

	x = (1 / x) * -1; // invert the slope

	b = y * p1.y - (x * p1.x); // find new b
	*/

	// using the point slope formula
	// source: https://www.youtube.com/watch?v=RQgJTo52UZw

	if (p2.x == p3.x) { // if the line p2, p3 is perpendicular to the y axis
		return LineIntersection(point(0, p1.y), p1, p2, p3);
	}
	else if (p2.y == p3.y) { // if the line p2, p3 is perpendicular to the x axis
		
		return LineIntersection(point(p1.x, 0), p1, p2, p3);
	}
	else {
		float m = (p2.y - p3.y) / (p2.x - p3.x); // slope
		float b = p2.y - m * p2.x; // not necessary

		float new_m = (1 / m) * -1; // get reciprocal slope
		float new_b = p1.y - new_m * p1.x; // get reciprocal slope

		point p;

		p.x = (0 - new_b) / new_m;
		p.y = 0;

		return LineIntersection(p, p1, p2, p3);
	}
}

GUI::GUI()
{
	// Name your application
	sAppName = "GUI";	
}

bool GUI::OnUserCreate()
{
	// set the initial offset
	pOffset.x = (-ScreenWidth() / 2) / fScale;
	pOffset.y = (-ScreenHeight() / 2) / fScale;

	return true;
}

bool GUI::OnUserUpdate(float fElapsedTime)
{	
	if (bInstructions) {
		if (GetKey(olc::I).bPressed) {
			bInstructions = false;
		}

		Clear(olc::BLUE);

		std::string msg = "";

		msg += "left mouse   - create polygon\n";
		msg += "scrool wheel - zoom\n";
		msg += "middle mouse - panning\n";
		msg += "z            - turn clipping on and off\n";
		msg += "x            - turn grid on and off\n";
		msg += "i            - open instructions menu\n";
		msg += "s            - save model\n";

		DrawString(1, 1, msg, olc::WHITE, 2);
	}
	else {	
		point pMouseScreen{ (float)GetMouseX(), (float)GetMouseY() };
		point pMouseWorld;
		ScreenToWorld(pMouseScreen, pMouseWorld);

		// get visibla world coords
		point pWorldTopLeft, pWorldBottomRight;
		ScreenToWorld(point(0, 0), pWorldTopLeft);
		ScreenToWorld(point(ScreenWidth(), ScreenHeight()), pWorldBottomRight);

		// Get values just beyond screen boundaries
		pWorldTopLeft.x = floor(pWorldTopLeft.x);
		pWorldTopLeft.y = floor(pWorldTopLeft.y);
		pWorldBottomRight.x = ceil(pWorldBottomRight.x);
		pWorldBottomRight.y = ceil(pWorldBottomRight.y);

		// handle user input

		// cursor clipping
		point pCursorScreen;
		if (bClipping == true) {
			point pMouseWorld_ = pMouseWorld;

			//float fDiffX = (pMouseWorld.x - floor(pMouseWorld.x)) - (fGrid);
			//float fDiffY = (pMouseWorld.y - floor(pMouseWorld.y)) - (fGrid);

			//pMouseWorld.x = (fDiffX < (fGrid / 2.0)) ? pMouseWorld.x - fDiffX : pMouseWorld.x + (fGrid / 2.0) - fDiffX; // clip on grid
			
			//pMouseWorld.y = (fDiffY < (fGrid / 2.0)) ? pMouseWorld.y - fDiffY : pMouseWorld.y + (fGrid / 2.0) - fDiffY; // clip on grid

			pMouseWorld.x = (pMouseWorld.x - floor(pMouseWorld.x) < (fGrid / 2.0)) ? floor(pMouseWorld.x) : ceil(pMouseWorld.x); // clip on grid
			pMouseWorld.y = (pMouseWorld.y - floor(pMouseWorld.y) < (fGrid / 2.0)) ? floor(pMouseWorld.y) : ceil(pMouseWorld.y); // clip on grid

			bShowCursor = true;
			WorldToScreen(pMouseWorld, pCursorScreen);
		}
		else {
			bShowCursor = false;

			// clip on other polygon point
			for (polygon& p : vPolygons) {
				for (int i = 0; i < p.vertices.size(); i++) {
					if (DistancePointToPoint(pMouseWorld, p.vertices[i]) <= fClippingRadius / fScale) {
						pMouseWorld = p.vertices[i];
						bShowCursor = true;
						break;
					}
				}
			}
			
			// clip on other polygons edge	
			if (bShowCursor == false) {
				for (polygon& p : vPolygons) {
					int i = 0;
					do {
						int next = (i + 1) % p.vertices.size();

						if (IsPointInLineSeg(pMouseWorld, p.vertices[i], p.vertices[next]) == true) {
							if (DistancePointToEdge(pMouseWorld, p.vertices[i], p.vertices[next]) < fClippingRadius / fScale) {
								//pMouseWorld = LineIntersection(pMouseWorld, point(pWorldBottomRight.x + 1, pMouseWorld.y), p.vertices[i], p.vertices[next]);
								pMouseWorld = ClosestPointFromLine(pMouseWorld, p.vertices[i], p.vertices[next]);
								bShowCursor = true;
							}
						}

						i = next;
					} while (i != 0);
				}
			}

			WorldToScreen(pMouseWorld, pCursorScreen);
		}

		// panning
		if (GetMouse(olc::Mouse::MIDDLE).bPressed) {
			pStartPan.x = pMouseScreen.x;
			pStartPan.y = pMouseScreen.y;
		}

		if (GetMouse(olc::Mouse::MIDDLE).bHeld) {
			pOffset.x -= (pMouseScreen.x - pStartPan.x) / fScale;
			pOffset.y -= (pMouseScreen.y - pStartPan.y) / fScale;

			pStartPan.x = pMouseScreen.x;
			pStartPan.y = pMouseScreen.y;
		}

		// zooming
		point pMouseWorldBeforeZoom;
		ScreenToWorld(pMouseScreen, pMouseWorldBeforeZoom);

		if (GetMouseWheel() > 0) {
			if (fScale <= fMaxScale)
				fScale *= 1.1f;

			if (fScale > fMaxScale)
				fScale = fMaxScale;
		}

		if (GetMouseWheel() < 0) {		
			if (fScale >= fMinScale)
				fScale *= 0.9f;

			if (fScale < fMinScale)
				fScale = fMinScale;
		}

		point pMouseWorldAfterZoom;
		ScreenToWorld(pMouseScreen, pMouseWorldAfterZoom);

		pOffset.x += (pMouseWorldBeforeZoom.x - pMouseWorldAfterZoom.x);
		pOffset.y += (pMouseWorldBeforeZoom.y - pMouseWorldAfterZoom.y);

		// exit
		if (GetKey(olc::CTRL).bHeld && GetKey(olc::Q).bPressed) {
			exit(1);
		}

		// toggle clipping
		if (GetKey(olc::Z).bPressed) {
			if (bClipping == false) {
				bClipping = true;
				bShowCursor = true;
			}			
			else {
				bClipping = false;
				bShowCursor = false;
			}
			
		}

		// toggle grid
		if (GetKey(olc::X).bPressed) {
			if (bShowGrid == false)
				bShowGrid = true;
			else
				bShowGrid = false;
		}

		// draw polygon
		if (GetMouse(olc::Mouse::LEFT).bPressed) {
			if (bCreatingPolygon == false)
				bCreatingPolygon = true;

			pNextPoint.x = pMouseWorld.x;
			pNextPoint.y = pMouseWorld.y;		

			pTempPolygon.vertices.push_back(pNextPoint);

			if (pTempPolygon.vertices.size() > 2) {
				if ((pTempPolygon.vertices.back().x == pTempPolygon.vertices.front().x && pTempPolygon.vertices.back().y == pTempPolygon.vertices.front().y) || DistancePointToPoint(pNextPoint, pTempPolygon.vertices.front()) < fClippingRadius / fScale) { //

				// remove last point from the polygon because it is the same as the first
					pTempPolygon.vertices.pop_back();

					// insert polygon
					vPolygons.push_back(pTempPolygon);

					// erase temp polygon if we are creating a new one
					pTempPolygon.vertices.clear();

					bCreatingPolygon = false;
				}
			}
			
		}

		// move point
		if (GetMouse(olc::Mouse::RIGHT).bHeld) {
			for (polygon& p : vPolygons) {
				for (int i = 0; i < p.vertices.size(); i++) {
					if (DistancePointToPoint(pMouseWorld, p.vertices[i]) < fClippingRadius / fScale) {
						//std::string s = std::to_string(p.vertices[i].x) + " " + std::to_string(p.vertices[i].y);
						//std::cout << s << std::endl;
						//DrawString(1, 10, s, olc::WHITE, 4);

						p.vertices[i].x += (pMouseWorld.x);
						p.vertices[i].y += (pMouseWorld.y);
					}
				}
			}
		}

		// erase last point
		if (GetKey(olc::ESCAPE).bPressed && bCreatingPolygon == true) {
			pTempPolygon.vertices.pop_back();

			if (pTempPolygon.vertices.size() == 0) {
				bCreatingPolygon = false;
			}
		}

		if (GetKey(olc::I).bPressed) {
			bInstructions = true;
		}

		// save model
		if (GetKey(olc::S).bPressed) {
			std::fstream out;
			out.open("polygons.txt", std::ios::out | std::ios::ate);

			if (out.is_open() == false) {
				std::cerr << "Erro saving model\n";
			}

			out << vPolygons.size() << std::endl;
			for (polygon& p : vPolygons) {
				out << p.vertices.size() << std::endl;

				for (point& po : p.vertices) {
					out << po.x << " " << po.y << std::endl;
				}
			}

			out.close();
		}
		// load model
		if (GetKey(olc::L).bPressed) {
			std::fstream in;
			in.open("polygons.txt", std::ios::in);

			if (in.is_open() == false) {
				std::cerr << "Erro loading model\n";
			}

			int iNumPolygons;
			int iNumPoints;

			in >> iNumPolygons;
			
			int i = 0;
			while (i < iNumPolygons) {
				in >> iNumPoints;
				polygon pl;
				int j = 0;
				while (j < iNumPoints) {
					point p;
					in >> p.x >> p.y;
					pl.vertices.push_back(p);
					j++;
				}
				vPolygons.push_back(pl);
				i++;
			}

			in.close();
		}

		// change grid size
		if (GetKey(olc::G).bPressed) {
			if (fGrid == 1.0f) {
				fGrid = 0.5f;
			}
			else {
				fGrid = 1.0f;
			}
		}

		// rendering

		// set background to dark blue
		Clear(olc::BLUE);	

		DrawString(1, 1, std::to_string(pMouseWorld.x) + " " + std::to_string(pMouseWorld.y), olc::WHITE);

		//DrawString(1, 3, std::to_string(fScale), olc::RED);

		//DrawString(1, 1, std::to_string(GetMouseX()) + " " + std::to_string(GetMouseY()), olc::WHITE, 2);

		if (bShowGrid) {
			// draw grid dots
			for (float x = pWorldTopLeft.x; x < pWorldBottomRight.x; x += fGrid) {
				for (float y = pWorldTopLeft.y; y < pWorldBottomRight.y; y += fGrid) {
					point pScreen;
					WorldToScreen(point{ x, y }, pScreen);
					Draw(pScreen.x, pScreen.y, olc::WHITE);
				}
			}

			// Draw axis
			point pAxisStart, pAxisEnd;

			// Y axis	
			WorldToScreen({ 0, pWorldTopLeft.y }, pAxisStart);
			WorldToScreen({ 0, pWorldBottomRight.y }, pAxisEnd);
			DrawLine(pAxisStart.x, pAxisStart.y, pAxisEnd.x, pAxisEnd.y, olc::WHITE, 0xF0F0F0F0);

			// X axis
			WorldToScreen({ pWorldTopLeft.x , 0 }, pAxisStart);
			WorldToScreen({ pWorldBottomRight.x, 0 }, pAxisEnd);
			DrawLine(pAxisStart.x, pAxisStart.y, pAxisEnd.x, pAxisEnd.y, olc::WHITE, 0xF0F0F0F0);
		}	

		// draw cursor
		if (bShowCursor)
			DrawCircle(pCursorScreen.x, pCursorScreen.y, fClippingRadius, olc::YELLOW);

		// draw temporary polygon
		if (pTempPolygon.vertices.size() > 0 && bCreatingPolygon == true) {
			point pCurr;
			point pNext;

			WorldToScreen(pTempPolygon.vertices[0], pCurr);

			DrawCircle(pCurr.x, pCurr.y, fClippingRadius, olc::RED);

			for (int i = 0; i < pTempPolygon.vertices.size() - 1; i++) {	
			
				WorldToScreen(pTempPolygon.vertices[i], pCurr);			
				WorldToScreen(pTempPolygon.vertices[i + 1], pNext);
				DrawLine(pCurr.x, pCurr.y, pNext.x, pNext.y, olc::GREY, 0xf0f0f0f0);
			}
			WorldToScreen(pTempPolygon.vertices.back(), pCurr);
			DrawLine(pCurr.x, pCurr.y, pCursorScreen.x, pCursorScreen.y, olc::GREY, 0xf0f0f0f0);
		}
	
		// draw polygons
		for (polygon p : vPolygons) {
			int i = 0;
			do {
				int next = (i + 1) % p.vertices.size();

				point pCurr;
				point pNext;

				WorldToScreen(p.vertices[i], pCurr);
				WorldToScreen(p.vertices[next], pNext);

				DrawLine(pCurr.x, pCurr.y, pNext.x, pNext.y, olc::WHITE);
				i = next;
			} while (i != 0);
		}
	}

	return true;
}

