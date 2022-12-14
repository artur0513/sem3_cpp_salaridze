#include <SFML/Graphics/Texture.hpp>

#include <type_traits>

static_assert(std::is_copy_constructible_v<sf::Texture>);
static_assert(std::is_copy_assignable_v<sf::Texture>);
static_assert(std::is_move_constructible_v<sf::Texture>);
static_assert(!std::is_nothrow_move_constructible_v<sf::Texture>);
static_assert(std::is_move_assignable_v<sf::Texture>);
static_assert(!std::is_nothrow_move_assignable_v<sf::Texture>);
