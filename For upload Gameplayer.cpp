#include<map>
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
            ActionDetermined(info);//未完成，後續行動決定，預計在裡面呼叫EnqueueAction
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
        myboard.resize(height, std::vector<int>(width, -9)); // 全部先設為-9
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
                if (background_ptr != nullptr) 
                {
                    if (auto num = dynamic_cast<const NumberCell*>(background_ptr)) 
                    {
                        int value = num->GetNumber()%divisor;
                        myboard[r][c] = -5 ;//數字標-5，表示礦
                        if(!value)
                        {
                            usable_mine.push_back({r,c});
                        }
                        else
                        {
                            bad_mine[value] = {r,c};
                        }
                    }
                }
            }
        }
    }
    void BuildMiningPath(const IGameInfo& info, CellPosition src, CellPosition dst) {

    }
    void ActionDetermined(const IGameInfo& info)
    {
        for (auto& mine : usable_mine) {
            BuildMiningPath(info, mine, factoryCenter); // 3. 為能整除的礦建構傳送帶
        }
        handle_bad_mine(info, bad_mine); // 4. 將不能整除的礦合成
    }
    void handle_bad_mine(const IGameInfo& info , std::map<int , CellPosition>& bad_mine)
    {
        
    }
    bool not_initialize = true ;
    std::queue<PlayerAction> actions_;
    bool initial = true;
    std::vector<std::vector<int>>
            myboard;
    int divisor ;
    CellPosition factoryCenter{
        GameManager::CollectionCenterConfig::kTop + GameManagerConfig::kGoalSize / 2,
        GameManager::CollectionCenterConfig::kLeft + GameManagerConfig::kGoalSize / 2
    };
    std::vector<CellPosition> usable_mine ;
    std::map<int , CellPosition> bad_mine ; 
};
