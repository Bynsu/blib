#pragma once

#include <blib/math/Rectangle.h>
#include <glm/glm.hpp>
#include <functional>


namespace blib
{
	class Texture;
	class SpriteBatch;
	class AnimatableSprite;

	class Animation
	{
	public:
		float duration;
		float elapsedTime;
		std::function<void()> onDone;

		Animation()
		{
			onDone = nullptr;
			duration = 1;
			elapsedTime = 0;
		}

		virtual void apply(AnimatableSprite* sprite) = 0;
		virtual void finish(AnimatableSprite* sprite) {};
	};


	class MoveToAnimation : public Animation
	{
		blib::math::Rectangle src;
		blib::math::Rectangle dest;
	public:
		MoveToAnimation(const blib::math::Rectangle& src, const blib::math::Rectangle& dest);
		virtual void apply(AnimatableSprite* sprite);
	};

	class ShakeAnimation : public Animation
	{
		blib::math::Rectangle original;
		bool done;
	public:
		ShakeAnimation(const blib::math::Rectangle& original);
		virtual void apply(AnimatableSprite* sprite);
		void finish(AnimatableSprite* sprite);;
	};


	class AnimatableSprite
	{
	public:
		blib::math::Rectangle rect;
		blib::Texture* texture;
		std::vector<Animation*> animations;



		AnimatableSprite(blib::Texture* texture, const blib::math::Rectangle &rect);
		AnimatableSprite(blib::Texture* texture, const glm::vec2 &pos);


		void moveTo(const glm::vec2 &targetPosition, float time, const std::function<void()> &onDone = nullptr);
		void resizeTo(const glm::vec2 &targetSize, float time, const std::function<void()> &onDone = nullptr);
		void moveTo(const blib::math::Rectangle &targetRect, float time, const std::function<void()> &onDone = nullptr);

		void setAnimation(Animation* animation);

		void draw(SpriteBatch* spriteBatch);
		void update(float elapsedTime);
		void shake();
		void clearAnimations();
	};
}