#include <list>

#include "word.hpp"
#include "character.hpp"


Word::Word(float fontSize)
{
    this->fontSize = fontSize;
}

void Word::addCharacter(Character* character)
{
    characters.push_back((*character));
    width += character->getxAdvance()*fontSize;
}

std::list<Character>* Word::getCharacters()
{
    return &characters;
}

float Word::getWordWidth()
{
    return width;
}
