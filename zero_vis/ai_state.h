#pragma once

class AiState
{
public:
    AiState(class AiComponent* owner) : owner_(owner)
    { }
    virtual void Update(float delta_time) = 0;
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual const char* GetName() const = 0;

protected:
    class AiComponent* owner_;
};

class AiPatrol : AiState
{
public:
    AiPatrol(class AiComponent* owner)
        : AiState(owner)
    { }

    void Update(float delta_time) override;
    void OnEnter() override;
    void OnExit() override;

    const char* GetName() const override
    { return "Patrol"; }
};

class AiDeath : AiState
{
public:
    AiDeath(class AiComponent* owner)
        : AiState(owner)
    { }

    void Update(float delta_time) override;
    void OnEnter() override;
    void OnExit() override;

    const char* GetName() const override
    { return "Death"; }
};

class AiAttack : AiState
{
public:
    AiAttack(class AiComponent* owner)
        : AiState(owner)
    { }

    void Update(float delta_time) override;
    void OnEnter() override;
    void OnExit() override;

    const char* GetName() const override
    { return "Attack"; }
};
