#include "Particle.h"

//--------------------------------------------------------------
Particle::Particle() {

	this->location = glm::vec2(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
	this->velocity = glm::vec2(ofRandom(-1, 1), ofRandom(-1, 1));

	this->range = 30;
	this->max_force = 1;
	this->max_speed = 8;

	ofColor color;
	vector<int> hex_list = { 0xef476f, 0xffd166, 0x06d6a0, 0x118ab2, 0x073b4c };
	vector<ofColor> base_color_list;
	for (auto hex : hex_list) {

		color.setHex(hex);
		base_color_list.push_back(color);
	}

	this->color = base_color_list[(int)ofRandom(base_color_list.size())];
}

//--------------------------------------------------------------
Particle::~Particle() {}

//--------------------------------------------------------------
void Particle::update(vector<unique_ptr<Particle>>& particles) {

	// •ª—£
	glm::vec2 separate = this->separate(particles);
	this->applyForce(separate);

	// ®—ñ
	glm::vec2 align = this->align(particles);
	this->applyForce(align);

	// Œ‹‡
	glm::vec2 cohesion = this->cohesion(particles);
	this->applyForce(cohesion);

	// Ž©‰ä
	if (glm::length(this->velocity) > 0) {

		glm::vec2 future = glm::normalize(this->velocity) * this->range;
		future += this->location;

		float angle = ofRandom(360);
		glm::vec2 target = future + glm::vec2(this->range * 0.5 * cos(angle * DEG_TO_RAD), this->range * 0.5 * sin(angle * DEG_TO_RAD));

		glm::vec2 ego = this->seek(target);
		this->applyForce(ego);
	}

	// ‹«ŠE
	if (glm::length(this->location - glm::vec2(ofGetWidth() * 0.5, ofGetHeight() * 0.5)) > 500) {

		glm::vec2 area = this->seek(glm::vec2(ofGetWidth() * 0.5, ofGetHeight() * 0.5));
		this->applyForce(area);
	}

	// ‘Oi
	this->velocity += this->acceleration;
	if (glm::length(this->velocity) > this->max_speed) {

		this->velocity = glm::normalize(this->velocity) * this->max_speed;
	}
	this->location += this->velocity;
	this->acceleration *= 0;
	this->velocity *= 0.98;

	// ‹L˜^
	this->log.push_back(this->location);
	while (this->log.size() > 10) {

		this->log.erase(this->log.begin());
	}
}

//--------------------------------------------------------------
void Particle::draw() {

	if (this->log.size() < 3) { return; }

	auto head_size = 3;
	ofMesh mesh;
	vector<glm::vec3> right, left;
	glm::vec3 last_location;
	float last_theta;

	for (int k = 0; k < this->log.size() - 1; k++) {

		auto loc = glm::vec3(this->log[k], 0);
		auto next = glm::vec3(this->log[k + 1], 0);

		auto direction = next - loc;
		auto theta = atan2(direction.y, direction.x);

		right.push_back(loc + glm::vec3(ofMap(k, 0, this->log.size(), 0, head_size) * cos(theta + PI * 0.5), ofMap(k, 0, this->log.size(), 0, head_size) * sin(theta + PI * 0.5), 0));
		left.push_back(loc + glm::vec3(ofMap(k, 0, this->log.size(), 0, head_size) * cos(theta - PI * 0.5), ofMap(k, 0, this->log.size(), 0, head_size) * sin(theta - PI * 0.5), 0));

		last_location = loc;
		last_theta = theta;
	}

	for (int k = 0; k < right.size(); k++) {

		mesh.addVertex(left[k]);
		mesh.addVertex(right[k]);

		mesh.addColor(ofColor(this->color, ofMap(k, 0, this->log.size(), 0, 255)));
		mesh.addColor(ofColor(this->color, ofMap(k, 0, this->log.size(), 0, 255)));
	}

	for (int k = 0; k < mesh.getNumVertices() - 2; k += 2) {

		mesh.addIndex(k + 0); mesh.addIndex(k + 1); mesh.addIndex(k + 3);
		mesh.addIndex(k + 0); mesh.addIndex(k + 2); mesh.addIndex(k + 3);
	}

	auto tmp_header_size = ofMap(this->log.size() - 2, 0, this->log.size(), 0, head_size);
	auto tmp_alpha = ofMap(this->log.size() - 2, 0, this->log.size(), 0, 255);

	mesh.addVertex(last_location);
	mesh.addColor(ofColor(this->color, tmp_alpha));

	int index = mesh.getNumVertices();
	for (auto theta = last_theta - PI * 0.5; theta <= last_theta + PI * 0.5; theta += PI / 20) {

		mesh.addVertex(last_location + glm::vec3(tmp_header_size * cos(theta), tmp_header_size * sin(theta), 0));
		mesh.addColor(ofColor(this->color, tmp_alpha));
	}

	for (int k = index; k < mesh.getNumVertices() - 1; k++) {

		mesh.addIndex(index); mesh.addIndex(k + 0); mesh.addIndex(k + 1);
	}

	mesh.draw();
}

//--------------------------------------------------------------
glm::vec2 Particle::separate(vector<unique_ptr<Particle>>& particles) {

	glm::vec2 result;
	glm::vec2 sum;
	int count = 0;
	for (auto& other : particles) {

		glm::vec2 difference = this->location - other->location;
		if (glm::length(difference) > 0 && glm::length(difference) < this->range * 0.5) {

			sum += glm::normalize(difference);
			count++;
		}
	}

	if (count > 0) {

		glm::vec2 avg = sum / count;
		avg = avg * this->max_speed;
		if (glm::length(avg) > this->max_speed) {

			avg = glm::normalize(avg) * this->max_speed;
		}
		glm::vec2 steer = avg - this->velocity;
		if (glm::length(steer) > this->max_force) {

			steer = glm::normalize(steer) * this->max_force;
		}
		result = steer;
	}

	return result;
}

//--------------------------------------------------------------
glm::vec2 Particle::align(vector<unique_ptr<Particle>>& particles) {

	glm::vec2 result;
	glm::vec2 sum;
	int count = 0;
	for (auto& other : particles) {

		glm::vec2 difference = this->location - other->location;
		if (glm::length(difference) > 0 && glm::length(difference) < this->range) {

			sum += other->velocity;
			count++;
		}
	}

	if (count > 0) {

		glm::vec2 avg = sum / count;
		avg = avg * this->max_speed;
		if (glm::length(avg) > this->max_speed) {

			avg = glm::normalize(avg) * this->max_speed;
		}
		glm::vec2 steer = avg - this->velocity;
		if (glm::length(steer) > this->max_force) {

			steer = glm::normalize(steer) * this->max_force;
		}
		result = steer;
	}

	return result;
}

//--------------------------------------------------------------
glm::vec2 Particle::cohesion(vector<unique_ptr<Particle>>& particles) {

	glm::vec2 result;
	glm::vec2 sum;
	int count = 0;
	for (auto& other : particles) {

		glm::vec2 difference = this->location - other->location;
		if (glm::length(difference) > 0 && glm::length(difference) < this->range * 0.5) {

			sum += other->location;
			count++;
		}
	}

	if (count > 0) {

		result = this->seek(sum / count);
	}

	return result;
}

//--------------------------------------------------------------
glm::vec2 Particle::seek(glm::vec2 target) {

	glm::vec2 desired = target - this->location;
	float distance = glm::length(desired);
	desired = glm::normalize(desired);
	desired *= distance < this->range ? ofMap(distance, 0, this->range, 0, this->max_speed) : max_speed;
	glm::vec2 steer = desired - this->velocity;
	if (glm::length(steer) > this->max_force) {

		steer = glm::normalize(steer) * this->max_force;
	}
	return steer;
}

//--------------------------------------------------------------
void Particle::applyForce(glm::vec2 force) {

	this->acceleration += force;
}