#pragma once

#include "constants.h"

#include "InputState.h"
#include "Player.h"

#include <glm/glm.hpp>
#include "/usr/local/include/noise/noise.h"

class MY_Camera {
  protected:
    glm::vec3 m_position{0.f};
    glm::vec3 m_focal_point{0.f};
    glm::mat4 m_view_matrix{1.f};

  public:
    MY_Camera() = default;

    [[nodiscard]] const glm::mat4& getViewMtx() const;
    [[nodiscard]] const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& position);
    [[nodiscard]] glm::mat4 getInverted(float pivotPoint);

    virtual void update(InputState& input) = 0;

    void look(const glm::vec3& at);
    void look(const glm::vec3& from, const glm::vec3& at);
};

class PlayerCamera : public MY_Camera {
  private:
    Player* m_player;
    float m_distance;
    float m_pitch;
    float m_angle_around;
    noise::module::Perlin noise;
    bool isshake=false;

    static constexpr float RESET_SPEED = constants::PI / 2.f;

  public:
    PlayerCamera(Player* player);
    void update(InputState& input) override;
    void shake();
    inline void SetShake(bool is) { this->isshake = is; }
};
