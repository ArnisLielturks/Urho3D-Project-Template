void Test()
{
    log.Info("[Helper.as] Test() 123");
}

void GivePointsToPlayer(Node@ playerNode, int amount)
{
    VariantMap data;
    data["Score"] = amount;
    playerNode.SendEvent("PlayerScoreAdd", data);
}