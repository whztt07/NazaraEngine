// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#pragma once

#ifndef NDK_BASEWIDGET_HPP
#define NDK_BASEWIDGET_HPP

#include <NDK/Prerequisites.hpp>
#include <NDK/Entity.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Platform/Event.hpp>
#include <Nazara/Platform/Mouse.hpp>
#include <Nazara/Utility/Node.hpp>
#include <limits>

namespace Ndk
{
	class Canvas;

	class NDK_API BaseWidget : public Nz::Node
	{
		friend Canvas;

		public:
			struct Padding;

			BaseWidget(BaseWidget* parent);
			BaseWidget(const BaseWidget&) = delete;
			BaseWidget(BaseWidget&&) = delete;
			virtual ~BaseWidget();

			template<typename T, typename... Args> T* Add(Args&&... args);
			inline void AddChild(std::unique_ptr<BaseWidget>&& widget);

			inline void Center();
			inline void CenterHorizontal();
			inline void CenterVertical();

			void ClearFocus();

			void Destroy();

			void EnableBackground(bool enable);

			//virtual BaseWidget* Clone() const = 0;

			inline const Nz::Color& GetBackgroundColor() const;
			inline Canvas* GetCanvas();
			inline Nz::SystemCursor GetCursor() const;
			inline const Padding& GetPadding() const;
			inline Nz::Vector2f GetContentOrigin() const;
			inline const Nz::Vector2f& GetContentSize() const;
			inline Nz::Vector2f GetSize() const;

			bool HasFocus() const;

			inline bool IsVisible() const;

			virtual void ResizeToContent() = 0;

			void SetBackgroundColor(const Nz::Color& color);
			void SetCursor(Nz::SystemCursor systemCursor);
			inline void SetContentSize(const Nz::Vector2f& size);
			void SetFocus();
			inline void SetPadding(float left, float top, float right, float bottom);
			void SetSize(const Nz::Vector2f& size);

			void Show(bool show = true);

			BaseWidget& operator=(const BaseWidget&) = delete;
			BaseWidget& operator=(BaseWidget&&) = delete;

			struct Padding
			{
				float left;
				float top;
				float right;
				float bottom;
			};

		protected:
			const EntityHandle& CreateEntity(bool isContentEntity);
			void DestroyEntity(Entity* entity);
			virtual void Layout();

			void InvalidateNode() override;

			virtual bool IsFocusable() const;
			virtual void OnFocusLost();
			virtual void OnFocusReceived();
			virtual bool OnKeyPressed(const Nz::WindowEvent::KeyEvent& key);
			virtual void OnKeyReleased(const Nz::WindowEvent::KeyEvent& key);
			virtual void OnMouseEnter();
			virtual void OnMouseMoved(int x, int y, int deltaX, int deltaY);
			virtual void OnMouseButtonPress(int x, int y, Nz::Mouse::Button button);
			virtual void OnMouseButtonRelease(int x, int y, Nz::Mouse::Button button);
			virtual void OnMouseExit();
			virtual void OnParentResized(const Nz::Vector2f& newSize);
			virtual void OnTextEntered(char32_t character, bool repeated);

		private:
			inline BaseWidget();

			inline void DestroyChild(BaseWidget* widget);
			void DestroyChildren();
			inline bool IsRegisteredToCanvas() const;
			inline void NotifyParentResized(const Nz::Vector2f& newSize);
			void RegisterToCanvas();
			inline void UpdateCanvasIndex(std::size_t index);
			void UnregisterFromCanvas();
			void UpdatePositionAndSize();

			struct WidgetEntity
			{
				EntityOwner handle;
				bool isContent;
			};

			static constexpr std::size_t InvalidCanvasIndex = std::numeric_limits<std::size_t>::max();

			std::size_t m_canvasIndex;
			std::vector<WidgetEntity> m_entities;
			std::vector<std::unique_ptr<BaseWidget>> m_children;
			Canvas* m_canvas;
			EntityOwner m_backgroundEntity;
			Padding m_padding;
			WorldHandle m_world;
			Nz::Color m_backgroundColor;
			Nz::SpriteRef m_backgroundSprite;
			Nz::SystemCursor m_cursor;
			Nz::Vector2f m_contentSize;
			BaseWidget* m_widgetParent;
			bool m_visible;
	};
}

#include <NDK/BaseWidget.inl>

#endif // NDK_BASEWIDGET_HPP
