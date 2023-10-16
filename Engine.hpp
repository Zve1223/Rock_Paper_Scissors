#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Windows.h>
#include <iostream>

#include <thread>
#include <chrono>

#define CHAR_SIZE 16u
#define LINE_SPACE 1.25f
#define MAX_SIZE_SOUND_POLL 140ull
#define ROCK 0u
#define PAPER 1u
#define SCISSORS 2u


namespace rps
{
	struct WindowConfig
	{
		unsigned int width = 720u;
		unsigned int height = 720u;
		std::string name = "Rock, Paper, Scissors";
		uint8_t antialiasingLevel = 16u;
	};


	struct GameSettings
	{
		size_t FPSLimit = 144ull;

		uint8_t types = 3u;
		size_t count = 32ull;
		float speed = 64.0f;
		float size = 32.0f;
		float volume = 50.0f;
	};


	class Engine
	{
	public:
		Engine();
		void run();

		~Engine();

	private:
		WindowConfig config;
		sf::ContextSettings settings;
		sf::RenderWindow* window;
		sf::Vector2u winSize;
		sf::Image icon;
		sf::Event event;
		bool isFullscreen;
		void setFullscreen();

		std::vector<std::vector<sf::RectangleShape*>> objects;
		sf::RectangleShape introPreview;

		sf::RectangleShape* createObject(sf::Vector2f, float, uint8_t);
		inline sf::Vector2f getRandomPos();

		long double deltaTime;
		std::chrono::high_resolution_clock timer;
		std::chrono::steady_clock::time_point startFrameTime;

		GameSettings gameSettings;

		size_t FPSLimit;
		uint8_t types;
		size_t count;
		float speed;
		float size;
		float volume;

		std::vector<std::string> textureNames;
		std::vector<sf::Texture*> textures;
		sf::Texture* errorTexture;

		std::vector<std::vector<std::string>> soundNames;
		std::vector<std::vector<sf::SoundBuffer*>> soundBuffers;
		std::vector<sf::Sound*> soundHeap;
		std::vector<sf::Sound*> soundPoll;
		sf::SoundBuffer introBuffer;
		sf::Sound* introSound;
		void playSound(sf::SoundBuffer*);
		void cleanSoundPoll();
		void clearSoundPoll();
		void clearSoundHeap();

		std::string fontName;
		sf::Font font;

		sf::Text FPSCounter;
		long double timeCounter;
		void setFPSCounter();

		std::vector<sf::Text> F3Menu;
		bool isF3Menu;
		void setF3Menu();
		void setF3MenuStats();
		float getHeightOfBottomPanel(std::string&);

		std::vector<sf::Text> controlsTab;
		bool isControlsTab;
		void setControlsTab();

		void debugLog(size_t, ...);
		sf::Text debugString;
		void setDebugString();

		void loadSettings();
		void loadPresets();
		void loadIcon();
		void loadTextures();
		void createErrorTexture();
		void loadSounds();
		void loadFont();

		size_t getNearestObject(const sf::Vector2f&, std::vector<sf::RectangleShape*>&);
		inline void moveTo(sf::RectangleShape*, const sf::Vector2f&, float);

		void addObject(uint8_t);
		void deleteObject(uint8_t);
		void changeVolume(float);
		void changeSpeed(float);
		void changeSize(float);
		void changeCount(int64_t);

		void setIntro();
		void playIntro();

		void restart();
		inline void clearEventPoll();
		inline void sleep(int64_t);
	};
}