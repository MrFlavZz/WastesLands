#pragma once

#ifndef DECOR_H
#define DECOR_H









#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
typedef boost::geometry::model::d2::point_xy<double> point;
typedef boost::geometry::model::polygon< point > polygon;


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::gl;
class Decor
{
public:
	Decor();
	

	void SetPositionX(float positionX);
	void SetPositionY(float positionY);
	float GetPositionX();
	float GetPositionY();

	void SetSizeX(float sizeX);
	void SeSizeY(float sizeY);
	float GetSizeX();
	float GetSizeY();

	void SetTexture(TextureRef texture);
	TextureRef GetTexture();

	void SetHitbox(polygon hitbox);
	polygon GetHitBox();

	void SetHitboxTexture(polygon hitBoxTexture) { this->hitBoxTexture = hitBoxTexture; }
	polygon GetHitBoxTexture() { return this->hitBoxTexture; }

	bool GetCollisionWithDecor() { return this->collisionWithDecor; }
	void SetCollisionithDecor(bool collisionWithDecor) { this->collisionWithDecor = collisionWithDecor; }
private:
	TextureRef texture;
	polygon hitbox;
	polygon hitBoxTexture;

	int positionX;
	int positionY;

	bool collisionWithDecor;

	float sizeX;
	float sizeY;
};

#endif