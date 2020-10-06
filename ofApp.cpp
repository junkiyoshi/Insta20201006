#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofSetWindowTitle("openFrameworks");

	ofBackground(239);

	for (int i = 0; i < 800; i++) {

		auto particle = make_unique<Particle>();
		this->particles.push_back(move(particle));
	}
}

//--------------------------------------------------------------
void ofApp::update() {

	for (auto& particle : this->particles) {

		particle->update(this->particles);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	for (auto& particle : this->particles) {

		particle->draw();
	}
}

//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}