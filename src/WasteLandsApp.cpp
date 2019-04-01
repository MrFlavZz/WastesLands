

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/algorithms/distance.hpp>
typedef boost::geometry::model::d2::point_xy<double> point;
typedef boost::geometry::model::polygon< point > polygon;

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "Personnage.h"
#include "cinder/qtime/QuickTimeGlImplAvf.h"
#include "Decor.h"
#include "Map.h"
#include "ProgressBar.h"
#include <Windows.h>
#include "Resources.h"
#include "Projectile.h"
#include "TextureToDraw.h"
#include "ciWMFVideoPlayer.h"
#include "Ennemies.h"
//0 Droite
//1 Haut
//2 gauche
//3 Bas



/* MAP 


0 = menu 
1=choix perso
2 = map ou on peux bouger / jouer
3 = credits 
4 = echap
5 = on est mouru



*/

//WalkL = BottomLeft LEft
//WalkR = Bottome BottomRight et Right
//WalktTR = Tr Top
//WalkTL = TL

// Aura vert = v�g�taux
// Jaune = recyclable ( verre , carton , plastique , conserve )
// Noir = non recycable
// Gris = ?



using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;








class WasteLands : public App {
public:
	void setup() override;
	WasteLands();
	void mouseDown(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void mouseMove(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void update() override;
	void draw() override;
	void resize() override;
	void drawTex(const ci::gl::Texture2dRef &tex, const ci::vec2 &pos, const Rectf & size);
	
	void SetPositionDecor();
	void SetCameraOutOfBound();
	void DrawPrincipalMenu();
	void DrawMainMap();

	void ClickOnPlay();
	void ClickOnCredits();

	void AttribAllImage(int pos);

	void ResizeButton();
	void DrawButton();

	polygon TransformHitBoxInOneray(polygon Polygon);
	void  DrawMenuDie();
	void drawProjectile(const ci::gl::Texture2dRef &tex, const ci::vec2 &pos, const Rectf & size, float orientation);
	void DrawHp();
private:
	std::map<int, bool> touchPressed;
	Personnage mainCharacter;

	vector <Projectile> allProjectile;

	int actualMap =0;
	int zoom = 1;
	Map currentMap;
	ProgressBar progressBarVar;
	vector <TextureToDraw> allThingToDraw;
	map<string, Ennemiesload> allEnnemiesLoad;
	vector<Ennemies> allEnnemies;
	ciWMFVideoPlayer * Video = NULL;
};

void DebugDrawPolygon(polygon polygon) {
	std::vector<point > points = polygon.outer();

	for (std::vector<point>::size_type i = 0; i < points.size(); ++i)
	{
		if (i + 1 != points.size()) {
			drawLine(vec2(points[i].x(), points[i].y()), vec2(points[i + 1].x(), points[i + 1].y()));
		}
		else {
			drawLine(vec2(points[i].x(), points[i].y()), vec2(points[0].x(), points[0].y()));
		}


	}
}

void WasteLands::ResizeButton() {
	for (auto & i : this->currentMap.GetAllButton()) {
		
		console() << getWindowWidth() << " " << this->currentMap.GetTextureCurrentMap()->getWidth() << endl;
		i.SetPosX((i.GetInitialPosX() * getWindowWidth()) / this->currentMap.GetTextureCurrentMap()->getWidth());
		i.SetPosY((i.GetInitialPosY()* getWindowHeight()) / this->currentMap.GetTextureCurrentMap()->getHeight());
		i.SetSizeX((i.GetInitialSizeX() * getWindowWidth()) / this->currentMap.GetTextureCurrentMap()->getWidth());
		i.SetSizeY((i.GetInitialSizeY()* getWindowHeight()) / this->currentMap.GetTextureCurrentMap()->getHeight());


		string tempStringPolygon = "POLYGON((";
		polygon tempPolygon;
		std::vector<point > points = i.GetInitialHitBox().outer();
		
		for (std::vector<point>::size_type j = 0; j < points.size(); ++j)
		{
			double temp1 = (points[j].x() * getWindowWidth()) / this->currentMap.GetTextureCurrentMap()->getWidth();
			double temp2 = (points[j].y()* getWindowHeight()) / this->currentMap.GetTextureCurrentMap()->getHeight();

			tempStringPolygon += to_string(temp1) + " " + to_string(temp2) + ",";

		}
		tempStringPolygon.pop_back();
		tempStringPolygon += "))";
	
		boost::geometry::read_wkt(
			tempStringPolygon, tempPolygon);
		i.SetHitbox( tempPolygon);
		
		
		
		
	}
	
}



float getAngle(const vec2 & target1, const vec2 & target2) {
	double angle = atan2(target1.y - target2.y, target1.x - target2.x);
	if (angle < 0) {
		angle += 2 * M_PI;
	}


	return angle;
}

WasteLands::WasteLands() {
	
}

void WasteLands::AttribAllImage(int pos) {
	this->currentMap.SetcurrentTextureMap(this->progressBarVar.GetCurrentTextureMap());
	switch (pos)
	{
	case 0:
		this->currentMap.SetAllButton(this->progressBarVar.GetAllButton());
		if (this->progressBarVar.GetLoadedMovie() == true) {
			this->Video = &this->progressBarVar.GetVideo();

			Video->play();
		}
		this->ResizeButton();
		break;
	case 2:
		this->actualMap = pos;
	
		this->currentMap.SetAllButton(this->progressBarVar.GetAllButton());
		if (this->progressBarVar.GetMainCharacter().GetAnimation().size() != 0) {
			this->mainCharacter = this->progressBarVar.GetMainCharacter();
			this->mainCharacter.SetActualAnimation();
			this->mainCharacter.SetPointerToAllProjectile(&this->allProjectile);
			this->mainCharacter.SetProjectile(this->progressBarVar.GetAllProjectileCharacter());
			this->mainCharacter.GetClockAnimation().start();
			this->mainCharacter.SetPosX(600);
			this->mainCharacter.SetPosY(600);
		}
		if (this->progressBarVar.GetAllDecor().size() != 0) {

			this->currentMap.SetDecor(this->progressBarVar.GetAllDecor());
			for (auto i : this->currentMap.GetDecor()) {
				TextureToDraw temp;

				temp.SetDistance(boost::geometry::distance(this->TransformHitBoxInOneray(i.GetHitBoxTexture()), point(this->currentMap.GetTextureCurrentMap()->getActualWidth(), this->currentMap.GetTextureCurrentMap()->getActualHeight()*getWindowWidth())));
				temp.SetPos(vec2(i.GetPositionX(), i.GetPositionY()));
				temp.SetSize(Rectf(0, 0, i.GetSizeX(), i.GetSizeY()));
				temp.SetTexture(i.GetTexture());
				this->allThingToDraw.push_back(temp);
			}
		}
		this->allEnnemiesLoad = this->progressBarVar.GetEnnemiesLoad();
		
		//this->allEnnemies.push_back(this->allEnnemiesLoad["Rose"].TransformEnnemiesLoadToEnnemies("Rose",this->allEnnemiesLoad["Rose"],600,600, &this->allProjectile,vec2(5,5),vec2(150,150),100,20));
		this->allEnnemies.push_back(this->allEnnemiesLoad["Rose"].TransformEnnemiesLoadToEnnemies("Rose", this->allEnnemiesLoad["Rose"], 550, 600, &this->allProjectile, vec2(5, 5), vec2(150, 150),100,20));
		this->mainCharacter.GetAura().GetClock().start();
		this->ResizeButton();
		break;
	default:
		break;
	}
	
	
	
	

	
	
		
	
	

	

}

void WasteLands::resize() {
	switch (this->actualMap)
	{
	case 0:

		this->ResizeButton();

		break;
	case 1:

		break;


	case 2:
	
		break;
	}

}

void WasteLands::SetCameraOutOfBound()
{

	mat4 view = getModelMatrix();
	if (this->getWindowWidth() / 2 >= this->mainCharacter.GetPosX()) {

		view[3][0] = 0;


	}
	else if (this->currentMap.GetTextureCurrentMap()->getWidth() <= this->mainCharacter.GetPosX() + this->getWindowWidth() / 2) {
		view[3][0] = -this->currentMap.GetTextureCurrentMap()->getWidth() + this->getWindowWidth();

	}
	else {

		view[3][0] = -this->mainCharacter.GetPosX() + this->getWindowWidth() / 2;
	}

	if (this->getWindowHeight() / 2 >= this->mainCharacter.GetPosY()) {

		view[3][1] = 0;


	}
	else if (this->currentMap.GetTextureCurrentMap()->getHeight() <= this->mainCharacter.GetPosY() + this->getWindowHeight() / 2) {
		view[3][1] = -this->currentMap.GetTextureCurrentMap()->getHeight() + this->getWindowHeight();

	}
	else {

		view[3][1] = -this->mainCharacter.GetPosY() + this->getWindowHeight() / 2;
	}


	gl::setModelMatrix(view);

}



void WasteLands::mouseMove(MouseEvent event) {
	switch (this->actualMap)
	{
	case 0:
		for (auto i : this->currentMap.GetAllButton()) {
			if (i.IsInButton(point(event.getX(), event.getY())) && i.GetActive()) {

			}
		}
		break;
	case 2:
	{
		
		event.setPos(getMousePos() - getWindowPos());
		double x;
		double y;
		if (this->getWindowWidth() / 2 >= this->mainCharacter.GetPosX()) {

			x = event.getX();


		}
		else if (this->currentMap.GetTextureCurrentMap()->getWidth() <= this->mainCharacter.GetPosX() + this->getWindowWidth() / 2) {
			x = this->currentMap.GetTextureCurrentMap()->getWidth() - this->getWindowWidth() + event.getX();

		}
		else {
			x = this->mainCharacter.GetPosX() - this->getWindowWidth() / 2 + event.getX();
			
		}

		if (this->getWindowHeight() / 2 >= this->mainCharacter.GetPosY()) {

			y = event.getY();


		}
		else if (this->currentMap.GetTextureCurrentMap()->getHeight() <= this->mainCharacter.GetPosY() + this->getWindowHeight() / 2) {
			
			y = this->currentMap.GetTextureCurrentMap()->getHeight() - this->getWindowHeight() + event.getY();
		}
		else {

			
			y = this->mainCharacter.GetPosY() - this->getWindowHeight() / 2 + event.getY();
		}
		double dx = (double)(cos(getAngle(vec2(this->mainCharacter.GetPosX(), this->mainCharacter.GetPosY()), vec2(x, y))));
		double dy = (double)(sin(getAngle(vec2(this->mainCharacter.GetPosX(), this->mainCharacter.GetPosY()),vec2(x,y))));
		this->mainCharacter.SetOrientation(vec2(-dx,- dy));
	}
		break;
	
	default:
		break;
	}


	
}

void WasteLands::ClickOnPlay() {
	

	




	std::experimental::filesystem::v1::path  path = getAssetDirectories()[0].generic_u8string() + "\\Map\\Spawn";



	this->progressBarVar.SetupMapCharacterAndEnnemies(path, "Archer",2);
	

	this->Video->close();
}
void  WasteLands::ClickOnCredits() {
	
}


void WasteLands::setup()
{
	setFrameRate(60);
	//setFullScreen(true);
	std::experimental::filesystem::v1::path chemin = getAssetDirectories()[0].u8string() + "\\Map\\PrincipalMenu";
	
	this->progressBarVar.SetupMenu(chemin,0);
	
	

}

void WasteLands::mouseDown(MouseEvent event)
{
	switch (this->actualMap)
	{
	case 0:
		for (auto i : this->currentMap.GetAllButton()) {
			if (i.IsInButton(point(event.getX(), event.getY())) && i.GetActive()) {
				if (i.GetTypeButton() == "ClickOnPlay()") {
					this->ClickOnPlay();
				}
			}
		}
		break;

	case 2:
		if (event.isRight()) {
			this->mainCharacter.GetAura().ChangeAura();
			
		}
		else if (event.isLeft()) {
			
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shot")] = true;
		}
		break;
	}
}


void WasteLands::mouseUp(MouseEvent event) {
	switch (this->actualMap){

	case 2:
		
		if (event.isLeft()) {
			
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shot")] = false;
		}
		
		break;
	}
}

void WasteLands::keyDown(KeyEvent event)
{
	
	switch (this->actualMap)
	{
	case 2:
		if (event.isShiftDown()) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] = true;

		}
		
		int touch = event.getCode();
		this->touchPressed[touch] = true;

		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkR")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunR")] = true;
			
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkL")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunL")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkT")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunT")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkB")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunB")] = true;

		}

		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkR")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkT")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTR")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkT")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkL")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTL")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkL")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkB")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBL")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkB")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkR")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBR")] = true;
		}

		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTR")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunTR")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTL")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunTL")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBL")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunBL")] = true;
		}
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBR")]) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunBR")] = true;

		}
		break;
	
	}
	

}
void WasteLands::keyUp(KeyEvent event)
{
	switch (this->actualMap)
	{
	case 2:
		
		if (event.isShiftDown() == false) {

			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] = false;
		}
		int touch = event.getCode();
	
		if (this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("ChangeAura")] == true && touch == this->mainCharacter.GetallTouches().GetValueTouche("ChangeAura")) {
			this->mainCharacter.GetAura().ChangeAura();
		}
		this->touchPressed[touch] = false;
		
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkR")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunR")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunR")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkL")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunL")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunL")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkT")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunT")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunT")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkB")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunB")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunB")] = false;
		}


		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkR")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkT")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTR")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTR")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkT")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkL")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTL")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTL")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkL")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkB")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBL")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBL")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkB")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkR")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBR")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBR")] = false;
		}

		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTR")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunTR")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunTR")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkTL")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunTL")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunTL")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBL")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunBL")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunBL")] = false;
		}
		if ((this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("Shift")] == false || this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("WalkBR")] == false) && this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunBR")] == true) {
			this->touchPressed[this->mainCharacter.GetallTouches().GetValueTouche("RunBR")] = false;
		}

		break;
	
	}
	


}





bool Collision(polygon first,polygon second) {
	bool temp;



	temp = boost::geometry::intersects(first, second);
	if (temp == true) {
		return true;
	}


	
	return false;

}




bool ValueCmp(TextureToDraw a, TextureToDraw b)
{
	return a.GetDistance() > b.GetDistance();
}

polygon WasteLands::TransformHitBoxInOneray(polygon Polygon) {
	polygon temp;
	string tempPoly = "POLYGON((";
	for (auto i : Polygon.outer()) {
		tempPoly += to_string(i.x());
		tempPoly += " ";
		tempPoly += to_string(i.y()*getWindowWidth());

		tempPoly += ",";
	}
	tempPoly.pop_back();
	tempPoly += "))";

	boost::geometry::read_wkt(
		tempPoly, temp);
	return temp;
}


void WasteLands::SetPositionDecor() {
	
	
	for (int posErase = 0; posErase < this->allThingToDraw.size(); posErase++) {
		if (this->allThingToDraw[posErase].GetDontMove() == false) {
			this->allThingToDraw.erase(this->allThingToDraw.begin() + posErase);
			posErase--;
		}
		
		
	}
	
	
	TextureToDraw temp;
	temp.SetDontMove(false);
	temp.SetDistance(boost::geometry::distance(this->TransformHitBoxInOneray(this->mainCharacter.GetHitBoxOnCurrentAnimation()), point(this->currentMap.GetTextureCurrentMap()->getActualWidth(), this->currentMap.GetTextureCurrentMap()->getActualHeight()*getWindowWidth())));
	temp.SetPos(vec2(this->mainCharacter.GetPosX(), this->mainCharacter.GetPosY()));
	temp.SetSize(Rectf(0, 0,this->mainCharacter.GetSizeX(), this->mainCharacter.GetSizeY()));
	temp.SetTexture(this->mainCharacter.GetActualAnimation().first);
	temp.SetSource("Main character");
	this->allThingToDraw.push_back(temp);

	for (int posErase = 0; posErase < this->allProjectile.size(); posErase++) {
		auto a = this->allProjectile[posErase];
	
		if (-a.GetStartTime() + getElapsedSeconds() > 5.0) {
			
			this->allProjectile.erase(this->allProjectile.begin() + posErase);
			posErase--;
		}
		else {
			TextureToDraw temp;
			temp.SetDontMove(false);
			temp.SetDistance(boost::geometry::distance(this->TransformHitBoxInOneray(a.GetActualHitBox()), point(this->currentMap.GetTextureCurrentMap()->getActualWidth(), this->currentMap.GetTextureCurrentMap()->getActualHeight()*getWindowWidth())));
			temp.SetPos(vec2(a.GetPosX(), a.GetPosY()));
			temp.SetSize(Rectf(0, 0, a.GetSizeX(), a.GetSizeY()));
			temp.SetTexture(a.GetTexture());
			temp.SetSource("Projectile");
			temp.SetOrientation(a.GetOrientation());
			this->allThingToDraw.push_back(temp);
		}
		
	}

	for (int posErase = 0; posErase < this->allEnnemies.size(); posErase++) {
		auto a = this->allEnnemies[posErase];
		TextureToDraw temp;
		temp.SetDontMove(false);
		temp.SetDistance(boost::geometry::distance(this->TransformHitBoxInOneray(a.GetActualHitbox()), point(this->currentMap.GetTextureCurrentMap()->getActualWidth(), this->currentMap.GetTextureCurrentMap()->getActualHeight()*getWindowWidth())));
		temp.SetPos(vec2(a.GetPos().x, a.GetPos().y));
		temp.SetSize(Rectf(0, 0, a.GetSize().x, a.GetSize().y));
		temp.SetTexture(a.GetCurrentTexture());
		temp.SetSource("Ennemies");
		this->allThingToDraw.push_back(temp);
		

	}


	std::sort(this->allThingToDraw.begin(), this->allThingToDraw.begin()+ this->allThingToDraw.size(), ValueCmp);



}



void WasteLands::update()
{
	

	if (this->progressBarVar.GetTerminated()) {
		switch (this->actualMap)
		{
		case 0:
			Video->update();
			break;
		case 2:
		{
			this->mouseMove(MouseEvent());
			vector <vec2> posFollowFrameEnnemi;
			
			bool temp;
			this->mainCharacter.Update(this->touchPressed);
			this->mainCharacter.SetActuelHitBoxOnCurrentAnimation();
			this->mainCharacter.GetAura().update();
			if (this->mainCharacter.GetIsDying() == 2) {
				this->actualMap = 5;
			}
			else {
				for (int j = 0; j < this->allEnnemies.size(); j++) {
					auto & a = this->allEnnemies[j];
					if (a.GetisDying() == 2) {
						this->allEnnemies.erase(this->allEnnemies.begin() + j);
						--j;
					}
					else {
						a.SetcurrentAnimation();
						a.Update(vec2(this->mainCharacter.GetPosX(), this->mainCharacter.GetPosY()));
						a.SetActualHitbox();
					}


				}

				for (int j = 0; j < this->allEnnemies.size(); j++) {
					auto & a = this->allEnnemies[j];


					for (auto i : this->currentMap.GetDecor()) {

						if (temp = Collision(a.GetActualHitbox(), i.GetHitBox())) {

							break;
						}
					}
					if (temp == false)
						for (auto i : this->allEnnemies) {

							if (a.GetPos() != i.GetPos()) {
								i.SetActualHitbox();
								if (temp = Collision(a.GetActualHitbox(), i.GetActualHitbox())) {
									break;
								}
							}
						}
					if (temp == false) {
						posFollowFrameEnnemi.push_back(vec2(a.GetPos()) + a.GetActualVelocity());

					}
					else {
						posFollowFrameEnnemi.push_back(vec2(a.GetPos()));
						a.SetActualVelocity(vec2(0, 0));
						a.SetActualHitbox();
					}

					a.SetActualVelocity(vec2(0, 0));

				}

				for (int j = 0; j < this->allEnnemies.size(); j++) {
					auto & a = this->allEnnemies[j];
					a.SetPos(posFollowFrameEnnemi[j]);
				}


				temp = false;

			
				if (this->mainCharacter.GetVelocityX() != 0 || this->mainCharacter.GetVelocityY() != 0) {
					for (auto i : this->currentMap.GetDecor()) {

						if (temp = Collision(this->mainCharacter.GetHitBoxOnCurrentAnimation(), i.GetHitBox())) {
							break;
						}
					}
					if (temp == false) {
						this->mainCharacter.SetPosX(this->mainCharacter.GetVelocityX() + this->mainCharacter.GetPosX());
						this->mainCharacter.SetPosY(this->mainCharacter.GetVelocityY() + this->mainCharacter.GetPosY());

					}
					this->mainCharacter.SetVelocityX(0);
					this->mainCharacter.SetVelocityY(0);
				}
				this->mainCharacter.SetActuelHitBoxOnCurrentAnimation();


				for (int j = 0; j < this->allProjectile.size(); j++) {
					bool temp = false;
					auto & a = this->allProjectile[j];
					a.SetAtualHitBox();
					for (auto i : this->currentMap.GetDecor()) {

						if (temp = Collision(a.GetActualHitBox(), i.GetHitBox())) {
							this->allProjectile.erase(this->allProjectile.begin() + j);
							--j;

							break;
						}
					}
					if (temp == false) {
						string sourceProjectile = a.GetSource();
						string type = sourceProjectile.substr(0, sourceProjectile.find(" "));


						if (type == "MainCharacter") {
							string aura = sourceProjectile.substr(sourceProjectile.find(" ") + 1);
							for (auto & i : this->allEnnemies) {
								if (temp = Collision(a.GetActualHitBox(), i.GetActualHitbox())) {
									if (aura == i.GetAuraKil()) {
										i.SetVie(i.GetVie() - a.GetDommage());
										if (i.GetVie() <= 0 && i.GetisDying() == 0) {
											i.SetDie();

										}

									}
									this->allProjectile.erase(this->allProjectile.begin() + j);
									--j;
									break;
								}
							}

						}
						else if (type == "Ennemies") {
							if (temp = Collision(a.GetActualHitBox(), this->mainCharacter.GetHitBoxOnCurrentAnimation())) {


								if (this->mainCharacter.GetVie() > 0) {
									this->mainCharacter.SetVie(this->mainCharacter.GetVie() - a.GetDommage());
									if (this->mainCharacter.GetVie() <= 0) {
										this->mainCharacter.SetVie(0);
										this->mainCharacter.SetEtatActuel("DieR");
										this->mainCharacter.SetCanChangeAnimationn(false);
										this->mainCharacter.SetIsDying(1);
									}
								}

								this->allProjectile.erase(this->allProjectile.begin() + j);
								--j;
							}
						}

					}

					if (temp == false) {
						a.SetPosX(a.GetPosX() + a.GetSpeedX());
						a.SetPosY(a.GetPosY() + a.GetSpeedY());

					}
				}


				this->SetPositionDecor();
			}
	
			

		}
			break;
		default:
			break;
		}
	}
	

	
	



}


void WasteLands::DrawHp() {
	gl::ScopedModelMatrix scpModel;
	
	double x ;
	double y;
	if (this->getWindowWidth() / 2 >= this->mainCharacter.GetPosX()) {

		x = 30;


	}
	else if (this->currentMap.GetTextureCurrentMap()->getWidth() <= this->mainCharacter.GetPosX() + this->getWindowWidth() / 2) {
		x = this->currentMap.GetTextureCurrentMap()->getWidth() - getWindowWidth() + 30;

	}
	else {
		x = this->mainCharacter.GetPosX() - getWindowWidth() / 2 + 30;

	}

	if (this->getWindowHeight() / 2 >= this->mainCharacter.GetPosY()) {

		y = 10;


	}
	else if (this->currentMap.GetTextureCurrentMap()->getHeight() <= this->mainCharacter.GetPosY() + this->getWindowHeight() / 2) {

		y = this->currentMap.GetTextureCurrentMap()->getHeight() - this->getWindowHeight() + 10;
	}
	else {


		y = this->mainCharacter.GetPosY() - getWindowHeight() / 2 + 10;
	}
	gl::translate(vec2(x, y));
	
	
	gl::color(Color(255, 0, 0));
	gl::drawSolidRect(Rectf(0, 0, (float)this->mainCharacter.GetVie() * 200 / this->mainCharacter.GetMaxVie(), 50));
	gl::drawStrokedRect({ 0, 0, 200, 50 });
	
	gl::color(1, 1, 1, 1);
}

void WasteLands::DrawMainMap() {

	

	gl::ScopedModelMatrix scpModel;
	gl::translate(getWindowCenter());
	gl::scale(vec2(this->zoom));
	
	this->SetCameraOutOfBound();
	gl::draw(this->currentMap.GetTextureCurrentMap());
	


	

	

	Rectf size(0, 0, this->mainCharacter.GetSizeX(), this->mainCharacter.GetSizeY());

	

	for (auto i : this->allThingToDraw) {
		if (i.GetSource() == "Projectile") {
			this->drawProjectile(i.GetTexture(), i.GetPos(), i.GetSize(), i.GetOrientation());
		}
		else if (i.GetSource() == "Main character") {
			this->drawTex(i.GetTexture(), i.GetPos(), i.GetSize());
			this->drawTex(this->mainCharacter.GetAura().GetTextureAura(), i.GetPos(), i.GetSize());
			
		}
		else {
			this->drawTex(i.GetTexture(), i.GetPos(), i.GetSize());
		}
	
		
	}

	this->DrawHp();

	//////////////////////////////DEBUG///////////////////

	for (auto a : this->allEnnemies) {
		DebugDrawPolygon(a.GetActualHitbox());
	}

	for (auto a : this->currentMap.GetDecor()) {
		DebugDrawPolygon(a.GetHitBox());
	}

	for (auto a : this->allProjectile) {
		DebugDrawPolygon(a.GetActualHitBox());
	}

	DebugDrawPolygon(this->mainCharacter.GetHitBoxOnCurrentAnimation());

}

void WasteLands::DrawButton() {
	gl::ScopedModelMatrix scpModel;
	for (auto i : this->currentMap.GetAllButton()) {
		Rectf sizeButton(0, 0, i.GetSizeX(), i.GetSizeY());
		vec2 pos = vec2(i.GetPosX(), i.GetPosY());
		
		gl::translate(pos);
		gl::draw(i.GetTexture(), sizeButton);
		gl::translate(-pos);
		
	

	}
	
}
void WasteLands::DrawMenuDie() {
	gl::ScopedModelMatrix scpModel;
	for (auto i : this->currentMap.GetAllButton()) {
		Rectf sizeButton(0, 0, i.GetSizeX(), i.GetSizeY());
		vec2 pos = vec2(i.GetPosX(), i.GetPosY());

		gl::translate(pos);
		gl::draw(i.GetTexture(), sizeButton);
		gl::translate(-pos);



	}
	

	
}
void WasteLands::DrawPrincipalMenu() {
	
	
	this->Video->draw(0, 0, getWindowWidth(), getWindowHeight());
	Rectf size(0, 0,getWindowWidth(), getWindowHeight());
	gl::draw(this->currentMap.GetTextureCurrentMap(), size);
	
	

	this->DrawButton();
	
}
void WasteLands::draw()
{
	
	if (this->progressBarVar.GetTerminated()) {
		
		gl::clear(Color(0, 0, 0));

		switch (this->actualMap)
		{
		case 0:
			this->DrawPrincipalMenu();
			break;
		case 2:
			this->DrawMainMap();
			break;
		case 5:
			this->DrawMenuDie();
			break;
		default:
			break;
		}
	}
	else {
		gl::clear(Color(0, 0, 0));
		if (this->progressBarVar.getProgress() < 1.0f) {
			// Draw the progress bar.
			gl::ScopedModelMatrix scpModel;

			const auto size = vec2(256, 16);
			gl::translate(getWindowCenter());
			gl::translate(-0.5f * size);
			gl::drawSolidRect({ 0, 0, this->progressBarVar.getProgress() * size.x, size.y });
			gl::drawStrokedRect({ 0, 0, size.x, size.y });
			// Draw an animated spinner.
			

			const float angle = float(getElapsedSeconds());
			const float step = glm::radians(30.0f);
			const float radius = 25.0f;
			
			gl::translate(vec2(0, 100));
			gl::translate(0.5f * size);
			for (int i = 0; i < 12; ++i) {
				gl::drawSolidCircle(radius * vec2(glm::sin(i * step - angle), glm::cos(i * step - angle)), 3.0f);
			}
		}
		else {
			this->progressBarVar.clean();
			this->AttribAllImage(this->progressBarVar.GetPos());
		}
		
	}
	

	gl::drawString("Framerate: " + to_string(getAverageFps()), vec2(0, 0), Color::white(), Font("Arial", 12));


}




void WasteLands::drawTex(const ci::gl::Texture2dRef &tex, const ci::vec2 &pos, const Rectf & size)
{
	gl::ScopedModelMatrix scpModel;
	gl::translate(pos - vec2(size.getWidth() / 2, size.getHeight() / 2));
	if (size.getHeight() == 0 && size.getWidth() == 0) {
		gl::draw(tex);
	}
	else {
		gl::draw(tex, size);
	}
	
}

void WasteLands::drawProjectile(const ci::gl::Texture2dRef &tex, const ci::vec2 &pos, const Rectf & size , float orientation)
{
	gl::ScopedModelMatrix scpModel;
	
	gl::translate(pos );
	gl::rotate(orientation );
	gl::translate(-vec2(size.getWidth() / 2, size.getHeight() / 2));
	
	
	
	if (size.getHeight() == 0 && size.getWidth() == 0) {
		gl::draw(tex);
	}
	else {
		gl::draw(tex, size);
	}
}



CINDER_APP(WasteLands, RendererGl(),
	[&](App::Settings *settings) {


})