#include "litehtmlContainer.h"

#include <cmath>
#include <string>

#include "../engine/console.h"
#include "../resources/css.h"
#include "../engine/engine.h"
#include "../basic/font.h"

render::LiteHTMLContainer::LiteHTMLContainer() {}

render::LiteHTMLContainer::~LiteHTMLContainer() {}

Text* render::LiteHTMLContainer::getText(Font* font, string input) {
  auto found = this->stringToText.find(std::pair(font, input));
  if(found == this->stringToText.end()) {
    Text* text = new Text(false);
    text->font = font;
    text->setText(input);
    this->stringToText[std::pair(font, input)] = {
      text: text,
      lastUsed: 0,
    };
    return text;
  }
  return found.value().text;
}

litehtml::uint_ptr render::LiteHTMLContainer::create_font(
  const litehtml::tchar_t* faceName,
  int size,
  int weight,
  litehtml::font_style italic,
  unsigned int decoration,
  litehtml::font_metrics* fm
) {
  Font* font = Font::GetFont(string(faceName), size);
  
  if(fm) {
    fm->ascent = font->ascent;
    fm->descent = font->descent;
    fm->height = font->ascent + font->descent;
    fm->x_height = font->x_height;
    fm->draw_spaces = false;
  }
  return (litehtml::uint_ptr)font;
}

void render::LiteHTMLContainer::delete_font(litehtml::uint_ptr hFont) {
  
}

int render::LiteHTMLContainer::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) {
  Font* font = (Font*)hFont;
  int x = 0;
  uint64_t length = strlen(text);
  for(uint64_t i = 0; i < length; i++) {
    FontGlyph ch = font->characterToGlyph[(unsigned char)text[i]]; // TODO sign check
    x += (ch.advance >> 6);
  }
  return x;
}

void render::LiteHTMLContainer::draw_text(
  litehtml::uint_ptr hdc,
  const litehtml::tchar_t* text,
  litehtml::uint_ptr hFont,
  litehtml::web_color color,
  const litehtml::position& pos
) {
  Text* foundText = this->getText((Font*)hFont, string(text));
  RenderContext context = {
		camera: engine->camera,
		ui: &engine->ui,
	};
  foundText->position.x = pos.left();
  if(foundText->position.x < 0) {
    foundText->position.x = pos.left() + engine->renderWindow.width;
  }
  
  foundText->position.y = pos.top() - ((Font*)hFont)->descent;
  foundText->color.r = (float)color.red / 255.0f;
  foundText->color.g = (float)color.green / 255.0f;
  foundText->color.b = (float)color.blue / 255.0f;
  foundText->render(0, context);
}

int render::LiteHTMLContainer::pt_to_px(int pt) const { return (int)((double)pt * 96 / 72.0); }

int render::LiteHTMLContainer::get_default_font_size() const { return 16; }

const litehtml::tchar_t* render::LiteHTMLContainer::get_default_font_name() const { return _t("Arial"); }

void render::LiteHTMLContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) {}

void render::LiteHTMLContainer::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) {
  if(this->sourceToImage[string(src)] != nullptr) {
    return;
  }

  std::string filename = "html";
  filename += src;
  resources::Image* image = (resources::Image*)(engine->manager->loadResources(engine->manager->carton->database.get()->equals("fileName", filename)->exec())[0]);
  this->sourceToImage[string(src)] = image;
}

void render::LiteHTMLContainer::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) {
  if(this->sourceToImage[string(src)] != nullptr) {
    sz.width = this->sourceToImage[string(src)]->getWidth();
    sz.height = this->sourceToImage[string(src)]->getHeight();
  }
}

void render::LiteHTMLContainer::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) {
  if(bg.color.alpha) {
    this->box.position.x = bg.clip_box.left();
    if(this->box.position.x < 0) {
      this->box.position.x = bg.clip_box.left() + engine->renderWindow.width;
    }

    this->box.position.y = bg.clip_box.y;
    this->box.size.x = bg.clip_box.width;
    this->box.size.y = bg.clip_box.height;
    this->box.color = glm::vec4((float)bg.color.red / 255.0, (float)bg.color.green / 255.0, (float)bg.color.blue / 255.0, (float)bg.color.alpha / 255.0);

    this->box.render();
  }
  
  resources::Image* image;
  if((image = this->sourceToImage[string(bg.image.c_str())]) != nullptr) {
    image->position.x = bg.position_x;
    image->position.y = bg.position_y;
    image->size.x = bg.image_size.width;
    image->size.y = bg.image_size.height;
    image->render();
  }
}

void render::LiteHTMLContainer::make_url(const litehtml::tchar_t* url, const litehtml::tchar_t* basepath, litehtml::tstring& out) {
  out = url;
}

void render::LiteHTMLContainer::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) {}

void render::LiteHTMLContainer::set_caption(const litehtml::tchar_t* caption) {}

void render::LiteHTMLContainer::set_base_url(const litehtml::tchar_t* base_url) {}

void render::LiteHTMLContainer::link(const std::shared_ptr<litehtml::document>& ptr, const litehtml::element::ptr& el) {}

void render::LiteHTMLContainer::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) {}

void render::LiteHTMLContainer::on_element_click(const litehtml::element::ptr& el) {
  litehtml::element::ptr element = el;
  
  while(element) {
    esObjectReferencePtr object = this->elementToESObject[&*element];
    if(object) {
      esEntry arguments[1];
      esCreateObjectAt(&arguments[0], object);
      esDeleteEntry(esCallMethod(engine->eggscript, object, "onClick", 1, arguments));
    }
    element = element->parent();
  }
}

void render::LiteHTMLContainer::set_cursor(const litehtml::tchar_t* cursor) {}

void render::LiteHTMLContainer::transform_text(litehtml::tstring& text, litehtml::text_transform tt) {}

void render::LiteHTMLContainer::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) {
  resources::CSS* css = (resources::CSS*)engine->manager->metadataToResources(
    engine->manager->carton->database.get()->equals("fileName", "html" + url)->exec()
  )[0];
  if(css != nullptr) {
    text = css->styles;
  }
}

void render::LiteHTMLContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) {}

void render::LiteHTMLContainer::del_clip() {}

void render::LiteHTMLContainer::get_client_rect(litehtml::position& client) const {
  client.x = 0;
  client.y = 0;
  client.width = engine->renderWindow.width;
  client.height = engine->renderWindow.height;
}

std::shared_ptr<litehtml::element> render::LiteHTMLContainer::create_element(
  const litehtml::tchar_t* tag_name,
  const litehtml::string_map& attributes,
  const std::shared_ptr<litehtml::document>& doc
) {
  return 0;
}

void render::LiteHTMLContainer::on_element_created(litehtml::element::ptr element) {
  engine->renderWindow.registerHTMLUpdate();

  std::string className = "HTMLElement";
  const char* attributeClassName = element->get_attr("es-class");
  if(attributeClassName != nullptr) {
    className = attributeClassName;
  }
  // TODO check hashing/equals of shared pointers
  esObjectReferencePtr test = this->elementToESObject[&*element] = esInstantiateObject(engine->eggscript, className.c_str(), this);

  if(test == nullptr) {
    console::error("could not create es html element with class %s\n", className.c_str());
  }
  else {
    this->esObjectToElement[test->objectWrapper] = element;
  }
  
  const char* id = element->get_attr("id");
  if(id != nullptr) {
    this->idToElement[id] = element;
  }
}

void render::LiteHTMLContainer::get_media_features(litehtml::media_features& media) const {
  litehtml::position client;
  get_client_rect(client);
  media.type = litehtml::media_type_screen;
  media.width = client.width;
  media.height = client.height;
  media.device_width = client.width;
  media.device_height = client.height;
  media.color = 8;
  media.monochrome = 0;
  media.color_index = 256;
  media.resolution = 96;
}

void render::LiteHTMLContainer::get_language(litehtml::tstring& language, litehtml::tstring& culture) const {
  language = _t("en");
  culture = _t("");
}

esObjectReferencePtr render::LiteHTMLContainer::getESObject(string id) {
  return this->elementToESObject[&*this->idToElement[id]];
}

esObjectReferencePtr render::LiteHTMLContainer::createChild(litehtml::element::ptr parent, string html) {
  esObjectReferencePtr array = esInstantiateObject(engine->eggscript, "Array", nullptr);
  litehtml::elements_vector vector = engine->renderWindow.htmlDocument->append_children_from_string(*parent, html.c_str());
  for(const auto& child: vector) {
    auto found = this->elementToESObject.find(&*child);
    esEntry entry;
    if(found != this->elementToESObject.end()) {
      esCreateObjectAt(&entry, found.value());
      esArrayPush(array, &entry);
    }
  }
  engine->renderWindow.registerHTMLUpdate();
  return array;
}
