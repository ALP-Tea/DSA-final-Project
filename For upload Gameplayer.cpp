class GamePlayer final : public Feis::IGamePlayer
{
public:
    PlayerAction GetNextAction(const IGameInfo &info) override 
    {
        if(not_initialize)
        {
            std::string divisor_string = info.GetLevelInfo();
            divisor = std::stoi(divisor_string.substr(1,divisor_string.size()-2));//得到mod用的數字
            Establish_map(info);//掃圖建立簡易
            ActionDetermined();//未完成，後續行動決定，預計在裡面呼叫EnqueueAction
            not_initialize = false ;//後面直接返回
        }
        if (actions_.empty()) {
            return {PlayerActionType::None, {0, 0}};
        }
        const PlayerAction action = actions_.front();
        actions_.pop();
        return action;
    }
    void EnqueueAction(const PlayerAction action) { actions_.push(action); }
private:
    void Establish_map(const IGameInfo& info) 
    {
        int height = GameManagerConfig::kBoardHeight;
        int width = GameManagerConfig::kBoardWidth;

        myboard.resize(height, std::vector<int>(width, 0)); // 全部先設為0
        int centerTop = GameManager::CollectionCenterConfig::kTop;
        int centerLeft = GameManager::CollectionCenterConfig::kLeft;
        int centerSize = GameManagerConfig::kGoalSize;

        for (int r = 0; r < height; ++r) 
        {
            for (int c = 0; c < width; ++c) 
            {
                // 工廠範圍標記
                if (r >= centerTop && r < centerTop + centerSize &&
                    c >= centerLeft && c < centerLeft + centerSize) 
                {
                    myboard[r][c] = -2;
                    continue;
                }
                CellPosition pos{r, c};
                const auto& cell = info.GetLayeredCell(pos); 
                // 判斷牆壁
                auto foreground_ptr = cell.GetForeground().get();
                if (foreground_ptr != nullptr &&
                    dynamic_cast<const WallCell*>(foreground_ptr) != nullptr) {
                    myboard[r][c] = -1;
                    continue;
                }
                // 數字礦石判斷
                auto background_ptr = cell.GetBackground().get();
                if (background_ptr != nullptr) {
                    if (auto num = dynamic_cast<const NumberCell*>(background_ptr)) {
                        myboard[r][c] = num->GetNumber();
                    }
                }
            }
        }
    }
    void ActionDetermined(){}
    bool not_initialize = true ;
    std::queue<PlayerAction> actions_;
    bool initial = true;
    std::vector<std::vector<int>>
            myboard;
    int divisor ;
};

