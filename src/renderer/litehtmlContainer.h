#pragma once

#include <eggscript/egg.h>
#include <litehtml.h>

#include "../renderer/box.h"
#include "dom.h"
#include "../resources/image.h"
#include "../basic/text.h"

namespace std {
	template<>
	struct hash<pair<class Font*, std::string>> {
		size_t operator()(pair<class Font*, std::string> const& source) const noexcept {
			uint64_t result = (uint64_t)source.first;
			return result ^ (hash<std::string>{}(source.second) + 0x9e3779b9 + (result << 6) + (result >> 2));
    }
	};
	
	template<>
	struct equal_to<pair<class Font*, std::string>> {
		bool operator()(const pair<class Font*, std::string>& x, const pair<class Font*, std::string>& y) const {
			return x.first == y.first && x.second == y.second;
		}
	};
};

namespace render {
	struct LiteHTMLTextWrapper {
		Text* text;
		uint64_t lastUsed;
	};
	
	class LiteHTMLContainer : public litehtml::document_container {
		friend esEntryPtr es::HTMLElement__getParent(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__addChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__removeChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__createChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__clear(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__setAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__getAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__setStyleAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		friend esEntryPtr es::HTMLElement__getStyleAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
		
		public:
			LiteHTMLContainer();
			virtual ~LiteHTMLContainer();

			virtual litehtml::uint_ptr create_font(
				const litehtml::tchar_t* faceName,
				int size,
				int weight,
				litehtml::font_style italic,
				unsigned int decoration,
				litehtml::font_metrics* fm
			) override;
			virtual void delete_font(litehtml::uint_ptr hFont) override;
			virtual int text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) override;
			virtual void draw_text(
				litehtml::uint_ptr hdc,
				const litehtml::tchar_t* text,
				litehtml::uint_ptr hFont, 
				litehtml::web_color color,
				const litehtml::position& pos
			) override;
			virtual int pt_to_px(int pt) const override;
			virtual int get_default_font_size() const override;
			virtual const litehtml::tchar_t*	get_default_font_name() const override;
			virtual void load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) override;
			virtual void get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) override;
			virtual void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) override;
			virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
			virtual void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
			virtual std::shared_ptr<litehtml::element>	create_element(
				const litehtml::tchar_t *tag_name,
				const litehtml::string_map &attributes,
				const std::shared_ptr<litehtml::document> &doc
			) override;
			virtual void on_element_created(litehtml::element::ptr el) override;
			virtual void get_media_features(litehtml::media_features& media) const override;
			virtual void get_language(litehtml::tstring& language, litehtml::tstring & culture) const override;
			virtual void link(const std::shared_ptr<litehtml::document> &ptr, const litehtml::element::ptr& el) override;


			virtual	void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
			virtual void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
			virtual void del_clip() override;

			virtual void make_url( const litehtml::tchar_t* url, const litehtml::tchar_t* basepath, litehtml::tstring& out );


			virtual	void set_caption(const litehtml::tchar_t* caption) override;
			virtual	void set_base_url(const litehtml::tchar_t* base_url) override;
			virtual void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
			virtual void on_element_click(const litehtml::element::ptr& el) override;
			virtual	void set_cursor(const litehtml::tchar_t* cursor) override;
			virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
			virtual void get_client_rect(litehtml::position& client) const override;

			esObjectReferencePtr getESObject(string id);
			esObjectReferencePtr createChild(litehtml::element::ptr parent, string html);
		
		private:
			Text text = Text(false);
			render::Box box;

			Text* getText(class Font* font, string text);
			tsl::robin_map<std::pair<class Font*, string>, LiteHTMLTextWrapper> stringToText;
			tsl::robin_map<string, resources::Image*> sourceToImage;
			tsl::robin_map<string, litehtml::element::ptr> idToElement;
			tsl::robin_map<litehtml::element*, esObjectReferencePtr> elementToESObject;
			tsl::robin_map<esObjectWrapperPtr, litehtml::element::ptr> esObjectToElement;
	};
};
