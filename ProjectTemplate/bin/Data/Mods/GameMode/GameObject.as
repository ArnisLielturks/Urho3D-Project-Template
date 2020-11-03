class GameObject : ScriptObject
{
    float duration;

    GameObject()
    {
        duration = -1; // Infinite
    }

    void FixedUpdate(float timeStep)
    {
        // Disappear when duration expired
        if (duration >= 0)
        {
            duration -= timeStep;
            if (duration <= 0) {
                node.Remove();
            }
        }
    }
}
