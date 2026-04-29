# graph-visualizer
My C++ and SFML project that shows graph algorithms like BFS, DFS, Dijkstra, Cycle Detection, and Connected Components step by step. It helps understand Data Structures and Algorithms easily through simple visuals and interactive graph traversal.
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <climits>
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>

using namespace std;

namespace Palette {
    const sf::Color BG            = sf::Color(15,  17,  26);
    const sf::Color PANEL_BG      = sf::Color(22,  26,  40, 230);
    const sf::Color PANEL_BORDER  = sf::Color(50,  60,  90);
    const sf::Color ACCENT        = sf::Color(90, 160, 255);
    const sf::Color ACCENT2       = sf::Color(120, 90, 240);
    const sf::Color SUCCESS       = sf::Color(60, 210, 130);
    const sf::Color WARNING       = sf::Color(255, 200,  60);
    const sf::Color DANGER        = sf::Color(255,  80,  80);
    const sf::Color TEXT_PRIMARY  = sf::Color(220, 228, 255);
    const sf::Color TEXT_MUTED    = sf::Color(120, 130, 160);
    const sf::Color NODE_DEFAULT  = sf::Color(35,  45,  75);
    const sf::Color NODE_VISITED  = sf::Color(40, 160, 100);
    const sf::Color NODE_ACTIVE   = sf::Color(220,  70,  70);
    const sf::Color NODE_QUEUED   = sf::Color(70, 120, 220);
    const sf::Color EDGE_DEFAULT  = sf::Color(55,  65,  95);
    const sf::Color EDGE_HIGHLIGHT= sf::Color(90, 160, 255);
    const sf::Color EDGE_TREE     = sf::Color(60, 210, 130);
    const sf::Color GRID          = sf::Color(25,  30,  48);
}

static float vecLen(sf::Vector2f v) { return std::sqrt(v.x*v.x + v.y*v.y); }
static sf::Vector2f vecNorm(sf::Vector2f v) {
    float l = vecLen(v);
    return l > 0 ? sf::Vector2f(v.x/l, v.y/l) : sf::Vector2f(0,0);
}

class Graph {
public:
    enum class NodeState { DEFAULT, QUEUED, VISITED, ACTIVE };
    struct EdgeState { bool highlighted = false; bool treeEdge = false; };

    int V;
    bool isDirected;
    vector<vector<pair<int,int>>> adj;
    vector<sf::Vector2f> pos;
    vector<NodeState> nodeState;
    vector<int> distances;
    vector<int> parents;
    vector<string> steps;
    int currentStep = 0;
    vector<vector<EdgeState>> edgeStates;

    Graph(int v, bool directed = false) : V(v), isDirected(directed) {
        resize();
        layoutCircle();
        pushStep("Graph created with " + to_string(V) + " vertices");
    }

    void resize() {
        adj.resize(V); pos.resize(V);
        nodeState.assign(V, NodeState::DEFAULT);
        distances.assign(V, INT_MAX);
        parents.assign(V, -1);
        edgeStates.resize(V);
    }

    void layoutCircle() {
        float cx = 430, cy = 330, r = 195;
        for (int i = 0; i < V; i++) {
            float a = 2.f * 3.14159f * i / V - 3.14159f/2.f;
            pos[i] = { cx + r*cosf(a), cy + r*sinf(a) };
        }
    }

    void addEdge(int u, int v, int w = 1) {
        if (u<0||u>=V||v<0||v>=V) return;
        adj[u].push_back({v, w});
        edgeStates[u].push_back({});
        if (!isDirected) {
            adj[v].push_back({u, w});
            edgeStates[v].push_back({});
        }
        pushStep("Edge added: " + to_string(u) + " - " + to_string(v) + "  (w=" + to_string(w) + ")");
    }

    void removeEdge(int u, int v) {
        auto erase = [&](int a, int b) {
            for (int i = (int)adj[a].size()-1; i >= 0; --i) {
                if (adj[a][i].first == b) {
                    adj[a].erase(adj[a].begin()+i);
                    edgeStates[a].erase(edgeStates[a].begin()+i);
                    break;
                }
            }
        };
        erase(u,v);
        if (!isDirected) erase(v,u);
        pushStep("Edge removed: " + to_string(u) + " - " + to_string(v));
    }

    void reset() {
        nodeState.assign(V, NodeState::DEFAULT);
        distances.assign(V, INT_MAX);
        parents.assign(V, -1);
        for (auto& es : edgeStates)
            for (auto& e : es) e = {};
        steps.clear(); currentStep = 0;
        pushStep("Ready");
    }

    void pushStep(const string& s) { steps.push_back(s); }
    void nextStep() { if (currentStep < (int)steps.size()-1) ++currentStep; }
    void prevStep() { if (currentStep > 0) --currentStep; }

    void BFS(int src) {
        reset();
        queue<int> q;
        nodeState[src] = NodeState::ACTIVE;
        distances[src] = 0;
        q.push(src);
        pushStep("BFS start at node " + to_string(src));

        while (!q.empty()) {
            int u = q.front(); q.pop();
            nodeState[u] = NodeState::VISITED;
            pushStep("Dequeue node " + to_string(u) + "  (dist=" + to_string(distances[u]) + ")");

            for (int i = 0; i < (int)adj[u].size(); ++i) {
                auto [v, w] = adj[u][i];
                if (nodeState[v] == NodeState::DEFAULT) {
                    nodeState[v] = NodeState::QUEUED;
                    distances[v] = distances[u] + 1;
                    parents[v] = u;
                    q.push(v);
                    edgeStates[u][i].treeEdge = true;
                    pushStep("Discovered " + to_string(v) + " via " + to_string(u));
                }
            }
        }
        pushStep("BFS complete");
    }

    void DFS(int src) {
        reset();
        pushStep("DFS start at node " + to_string(src));
        function<void(int,int)> dfs = [&](int u, int par) {
            nodeState[u] = NodeState::ACTIVE;
            pushStep("Enter node " + to_string(u));
            for (int i = 0; i < (int)adj[u].size(); ++i) {
                auto [v, w] = adj[u][i];
                if (nodeState[v] == NodeState::DEFAULT) {
                    parents[v] = u;
                    edgeStates[u][i].treeEdge = true;
                    pushStep("Tree edge: " + to_string(u) + " -> " + to_string(v));
                    dfs(v, u);
                } else if (v != par) {
                    edgeStates[u][i].highlighted = true;
                    pushStep("Back edge: " + to_string(u) + " -> " + to_string(v));
                }
            }
            nodeState[u] = NodeState::VISITED;
            pushStep("Leave node " + to_string(u));
        };
        dfs(src, -1);
        pushStep("DFS complete");
    }

    void dijkstra(int src) {
        reset();
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
        distances[src] = 0;
        nodeState[src] = NodeState::QUEUED;
        pq.push({0, src});
        pushStep("Dijkstra start at node " + to_string(src));

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (nodeState[u] == NodeState::VISITED) continue;
            nodeState[u] = NodeState::VISITED;
            pushStep("Settle node " + to_string(u) + "  dist=" + to_string(d));

            for (int i = 0; i < (int)adj[u].size(); ++i) {
                auto [v, w] = adj[u][i];
                if (distances[u] + w < distances[v]) {
                    distances[v] = distances[u] + w;
                    parents[v] = u;
                    nodeState[v] = NodeState::QUEUED;
                    pq.push({distances[v], v});
                    edgeStates[u][i].treeEdge = true;
                    pushStep("Relax " + to_string(v) + ": dist=" + to_string(distances[v]));
                }
            }
        }
        pushStep("Dijkstra complete");
    }

    bool detectCycle() {
        reset();
        vector<bool> recStack(V, false);
        bool found = false;
        pushStep("Cycle detection starting");

        function<bool(int,int)> dfs = [&](int u, int par) -> bool {
            nodeState[u] = NodeState::ACTIVE;
            recStack[u] = true;
            pushStep("Visit " + to_string(u));
            for (int i = 0; i < (int)adj[u].size(); ++i) {
                auto [v, w] = adj[u][i];
                if (nodeState[v] == NodeState::DEFAULT) {
                    if (dfs(v, u)) return true;
                } else if (v != par && recStack[v]) {
                    edgeStates[u][i].highlighted = true;
                    pushStep("Back edge " + to_string(u) + "->" + to_string(v) + " CYCLE!");
                    return true;
                }
            }
            nodeState[u] = NodeState::VISITED;
            recStack[u] = false;
            return false;
        };

        for (int i = 0; i < V; ++i)
            if (nodeState[i] == NodeState::DEFAULT)
                if (dfs(i, -1)) { found = true; break; }

        pushStep(found ? "Cycle found" : "No cycle found");
        return found;
    }

    vector<vector<int>> connectedComponents() {
        reset();
        vector<vector<int>> comps;
        pushStep("Connected components");

        for (int i = 0; i < V; ++i) {
            if (nodeState[i] != NodeState::DEFAULT) continue;
            vector<int> comp;
            queue<int> q;
            q.push(i);
            nodeState[i] = NodeState::QUEUED;

            while (!q.empty()) {
                int u = q.front(); q.pop();
                nodeState[u] = NodeState::VISITED;
                comp.push_back(u);
                pushStep("Node " + to_string(u) + " -> component " + to_string((int)comps.size()));
                for (int j = 0; j < (int)adj[u].size(); ++j) {
                    auto [v, w] = adj[u][j];
                    if (nodeState[v] == NodeState::DEFAULT) {
                        nodeState[v] = NodeState::QUEUED;
                        q.push(v);
                        edgeStates[u][j].treeEdge = true;
                    }
                }
            }
            comps.push_back(comp);
            pushStep("Component " + to_string((int)comps.size()-1) + ": " + to_string((int)comp.size()) + " nodes");
        }
        pushStep("Found " + to_string((int)comps.size()) + " component(s)");
        return comps;
    }

    void topoSort() {
        if (!isDirected) { pushStep("Topo sort requires directed graph"); return; }
        reset();
        pushStep("Topological sort starting");
        vector<int> order;
        function<void(int)> dfs = [&](int u) {
            nodeState[u] = NodeState::ACTIVE;
            pushStep("Visit " + to_string(u));
            for (int i = 0; i < (int)adj[u].size(); ++i) {
                auto [v, w] = adj[u][i];
                if (nodeState[v] == NodeState::DEFAULT) {
                    edgeStates[u][i].treeEdge = true;
                    dfs(v);
                }
            }
            nodeState[u] = NodeState::VISITED;
            order.push_back(u);
        };
        for (int i = 0; i < V; ++i)
            if (nodeState[i] == NodeState::DEFAULT) dfs(i);
        reverse(order.begin(), order.end());
        string s = "Order: ";
        for (int x : order) s += to_string(x) + " ";
        pushStep(s);
        pushStep("Topological sort complete");
    }

    int getV() const { return V; }
};

static void drawPanel(sf::RenderWindow& w, sf::Vector2f pos, sf::Vector2f size) {
    sf::RectangleShape p(size);
    p.setPosition(pos);
    p.setFillColor(Palette::PANEL_BG);
    p.setOutlineThickness(1.5f);
    p.setOutlineColor(Palette::PANEL_BORDER);
    w.draw(p);
}

class GraphRenderer {
    sf::RenderWindow& win;
    sf::Font& font;
    Graph& g;

    void drawArrow(sf::Vector2f from, sf::Vector2f to, sf::Color col) {
        sf::Vector2f dir = to - from;
        float len = vecLen(dir);
        if (len < 1.f) return;
        sf::Vector2f n = vecNorm(dir);
        float nodeR = 22.f;
        sf::Vector2f start = from + n * nodeR;
        sf::Vector2f end   = to   - n * nodeR;

        sf::Vertex line[] = { {start, col}, {end, col} };
        win.draw(line, 2, sf::Lines);

        sf::Vector2f perp(-n.y, n.x);
        float ah = 10.f, aw = 5.f;
        sf::ConvexShape head(3);
        head.setPoint(0, end);
        head.setPoint(1, end - n*ah + perp*aw);
        head.setPoint(2, end - n*ah - perp*aw);
        head.setFillColor(col);
        win.draw(head);
    }

    void drawGlow(sf::Vector2f center, float r, sf::Color col) {
        for (int i = 4; i >= 1; --i) {
            sf::CircleShape c(r + i*4.f);
            c.setOrigin(r + i*4.f, r + i*4.f);
            c.setPosition(center);
            sf::Color gc = col; gc.a = 15 * i;
            c.setFillColor(gc);
            win.draw(c);
        }
    }

public:
    GraphRenderer(sf::RenderWindow& w, sf::Font& f, Graph& gr) : win(w), font(f), g(gr) {}

    void draw() {
        drawGrid();
        drawEdges();
        drawNodes();
    }

    void drawGrid() {
        int step = 50;
        for (int x = 0; x < 900; x += step) {
            sf::Vertex l[] = {{sf::Vector2f((float)x,0), Palette::GRID},
                              {sf::Vector2f((float)x,700), Palette::GRID}};
            win.draw(l, 2, sf::Lines);
        }
        for (int y = 0; y < 700; y += step) {
            sf::Vertex l[] = {{sf::Vector2f(0,(float)y), Palette::GRID},
                              {sf::Vector2f(860,(float)y), Palette::GRID}};
            win.draw(l, 2, sf::Lines);
        }
    }

    void drawEdges() {
        for (int u = 0; u < g.V; ++u) {
            for (int i = 0; i < (int)g.adj[u].size(); ++i) {
                int v = g.adj[u][i].first;
                int w = g.adj[u][i].second;
                auto& es = g.edgeStates[u][i];

                sf::Color col = Palette::EDGE_DEFAULT;
                if (es.treeEdge)    col = Palette::EDGE_TREE;
                if (es.highlighted) col = Palette::DANGER;

                sf::Vector2f midP = (g.pos[u] + g.pos[v]) * 0.5f;

                if (es.treeEdge || es.highlighted) {
                    sf::Vector2f dir = g.pos[v] - g.pos[u];
                    sf::Vector2f n = vecNorm(dir);
                    sf::Vector2f p(-n.y, n.x);
                    float nodeR = 22.f;
                    sf::Vector2f s = g.pos[u] + n*nodeR;
                    sf::Vector2f e = g.pos[v] - n*nodeR;
                    sf::ConvexShape quad(4);
                    quad.setPoint(0, s + p*2.f); quad.setPoint(1, s - p*2.f);
                    quad.setPoint(2, e - p*2.f); quad.setPoint(3, e + p*2.f);
                    quad.setFillColor(col);
                    win.draw(quad);
                } else {
                    sf::Vertex line[] = {
                        {g.pos[u], col}, {g.pos[v], col}
                    };
                    win.draw(line, 2, sf::Lines);
                }

                if (g.isDirected) drawArrow(g.pos[u], g.pos[v], col);

                sf::Text wt;
                wt.setFont(font); wt.setString(to_string(w));
                wt.setCharacterSize(12); wt.setFillColor(Palette::TEXT_MUTED);
                sf::FloatRect b = wt.getLocalBounds();
                wt.setOrigin(b.width/2, b.height/2);
                wt.setPosition(midP + sf::Vector2f(0, -10));
                win.draw(wt);
            }
        }
    }

    void drawNodes() {
        float r = 22.f;
        for (int i = 0; i < g.V; ++i) {
            sf::Color fill, ring;
            switch (g.nodeState[i]) {
                case Graph::NodeState::ACTIVE:  fill = Palette::NODE_ACTIVE;  ring = Palette::DANGER;       break;
                case Graph::NodeState::VISITED: fill = Palette::NODE_VISITED; ring = Palette::SUCCESS;      break;
                case Graph::NodeState::QUEUED:  fill = Palette::NODE_QUEUED;  ring = Palette::ACCENT;       break;
                default:                        fill = Palette::NODE_DEFAULT; ring = Palette::PANEL_BORDER; break;
            }

            if (g.nodeState[i] != Graph::NodeState::DEFAULT)
                drawGlow(g.pos[i], r, ring);

            sf::CircleShape shadow(r+2);
            shadow.setOrigin(r+2,r+2); shadow.setPosition(g.pos[i]+sf::Vector2f(3,3));
            shadow.setFillColor(sf::Color(0,0,0,80));
            win.draw(shadow);

            sf::CircleShape circle(r);
            circle.setOrigin(r,r); circle.setPosition(g.pos[i]);
            circle.setFillColor(fill);
            circle.setOutlineThickness(2.5f); circle.setOutlineColor(ring);
            win.draw(circle);

            sf::Text idTxt;
            idTxt.setFont(font); idTxt.setString(to_string(i));
            idTxt.setCharacterSize(16); idTxt.setStyle(sf::Text::Bold);
            idTxt.setFillColor(Palette::TEXT_PRIMARY);
            sf::FloatRect tb = idTxt.getLocalBounds();
            idTxt.setOrigin(tb.left + tb.width/2, tb.top + tb.height/2);
            idTxt.setPosition(g.pos[i]);
            win.draw(idTxt);

            if (g.distances[i] < INT_MAX) {
                sf::Text dt;
                dt.setFont(font); dt.setString("d:" + to_string(g.distances[i]));
                dt.setCharacterSize(11); dt.setFillColor(Palette::WARNING);
                sf::FloatRect db = dt.getLocalBounds();
                dt.setOrigin(db.width/2, 0);
                dt.setPosition(g.pos[i].x, g.pos[i].y + r + 4);
                win.draw(dt);
            }
        }
    }
};

struct Button {
    sf::RectangleShape rect;
    sf::Text label;
    sf::Color normalColor, hoverColor, activeColor;
    bool hovered = false, pressed = false;
    std::function<void()> onClick;

    Button() {}
    Button(sf::Vector2f pos, sf::Vector2f size, const string& text, sf::Font& font,
           sf::Color nc, sf::Color hc, sf::Color ac, std::function<void()> cb)
        : normalColor(nc), hoverColor(hc), activeColor(ac), onClick(cb)
    {
        rect.setPosition(pos); rect.setSize(size);
        rect.setOutlineThickness(1.5f); rect.setOutlineColor(sf::Color(80,90,130));
        label.setFont(font); label.setString(text);
        label.setCharacterSize(13); label.setFillColor(Palette::TEXT_PRIMARY);
        sf::FloatRect b = label.getLocalBounds();
        label.setOrigin(b.left+b.width/2, b.top+b.height/2);
        label.setPosition(pos.x+size.x/2, pos.y+size.y/2);
    }

    void update(sf::Vector2i mouse, bool clicked) {
        hovered = rect.getGlobalBounds().contains((float)mouse.x, (float)mouse.y);
        if (hovered && clicked && onClick) onClick();
        rect.setFillColor(hovered ? hoverColor : normalColor);
    }

    void draw(sf::RenderWindow& w) { w.draw(rect); w.draw(label); }
};

class UIManager {
    sf::RenderWindow& win;
    sf::Font& font;
    Graph& g;

    vector<Button> algoBtns;
    Button btnPrev, btnNext, btnReset, btnAutoPlay;
    Button btnAddEdge, btnRemoveEdge;

    int edgeU = 0, edgeV = 1, edgeW = 1;
    int startNode = 0;
    int selAlgo = 0;
    bool autoPlay = false;
    float autoTimer = 0.f;
    float autoSpeed = 1.0f;

    const vector<string> algoNames = {"BFS","DFS","Dijkstra","Cycle","Components","Topo Sort"};

    void buildButtons() {
        float bx = 870, by = 30, bw = 190, bh = 34, gap = 10;

        for (int i = 0; i < (int)algoNames.size(); ++i) {
            int idx = i;
            algoBtns.push_back(Button(
                {bx, by + i*(bh+gap)}, {bw, bh}, algoNames[i], font,
                sf::Color(30,38,60), Palette::ACCENT2, Palette::ACCENT,
                [this, idx]() { selAlgo = idx; runAlgorithm(); }
            ));
        }

        float cy = by + 6*(bh+gap) + 20;

        btnPrev    = Button({bx,      cy},    {88,32}, "< Prev",  font, sf::Color(30,38,60), sf::Color(50,60,100), sf::Color(60,80,140),  [this](){ g.prevStep(); });
        btnNext    = Button({bx+98,   cy},    {88,32}, "Next >",  font, sf::Color(30,38,60), sf::Color(50,60,100), sf::Color(60,80,140),  [this](){ g.nextStep(); });
        btnReset   = Button({bx,      cy+42}, {88,32}, "Reset",   font, sf::Color(50,20,20),  Palette::DANGER,     sf::Color(180,50,50),  [this](){ g.reset(); autoPlay=false; });
        btnAutoPlay= Button({bx+98,   cy+42}, {88,32}, "Auto",    font, sf::Color(20,50,30), sf::Color(30,120,60), Palette::SUCCESS,      [this](){ autoPlay = !autoPlay; });

        float ey = cy + 110;
        btnAddEdge    = Button({bx, ey},    {190,32}, "Add Edge",    font, sf::Color(20,45,30), sf::Color(30,100,60), Palette::SUCCESS,     [this](){ g.addEdge(edgeU, edgeV, edgeW); });
        btnRemoveEdge = Button({bx, ey+42}, {190,32}, "Remove Edge", font, sf::Color(50,20,20), Palette::DANGER,     sf::Color(170,50,50), [this](){ g.removeEdge(edgeU, edgeV); });
    }

    void runAlgorithm() {
        switch (selAlgo) {
            case 0: g.BFS(startNode);        break;
            case 1: g.DFS(startNode);        break;
            case 2: g.dijkstra(startNode);   break;
            case 3: g.detectCycle();         break;
            case 4: g.connectedComponents(); break;
            case 5: g.topoSort();            break;
        }
    }

    void drawText(const string& s, float x, float y, int sz, sf::Color c, bool bold=false) {
        sf::Text t; t.setFont(font); t.setString(s);
        t.setCharacterSize(sz); t.setFillColor(c);
        if (bold) t.setStyle(sf::Text::Bold);
        t.setPosition(x, y);
        win.draw(t);
    }

    void drawSeparator(float x, float y, float w) {
        sf::Vertex line[] = { {sf::Vector2f(x,y), Palette::PANEL_BORDER},
                              {sf::Vector2f(x+w,y), Palette::PANEL_BORDER} };
        win.draw(line, 2, sf::Lines);
    }

    void drawSpinner(float x, float y, float val, float minV, float maxV,
                     const string& lbl, sf::Color col) {
        drawText(lbl, x, y, 12, Palette::TEXT_MUTED);

        sf::RectangleShape minus({16,16});
        minus.setPosition(x+70, y);
        minus.setFillColor(sf::Color(50,55,80));
        minus.setOutlineThickness(1); minus.setOutlineColor(Palette::PANEL_BORDER);
        win.draw(minus);
        drawText("-", x+75, y-1, 14, Palette::TEXT_PRIMARY);

        drawText(to_string((int)val), x+92, y, 13, col, true);

        sf::RectangleShape plus({16,16});
        plus.setPosition(x+112, y);
        plus.setFillColor(sf::Color(50,55,80));
        plus.setOutlineThickness(1); plus.setOutlineColor(Palette::PANEL_BORDER);
        win.draw(plus);
        drawText("+", x+115, y-1, 14, Palette::TEXT_PRIMARY);
    }

    int spinnerClick(sf::Vector2i mp, float x, float y) {
        sf::FloatRect minus(x+70, y, 16, 16);
        sf::FloatRect plus(x+112, y, 16, 16);
        if (minus.contains((float)mp.x,(float)mp.y)) return -1;
        if (plus.contains((float)mp.x,(float)mp.y))  return  1;
        return 0;
    }

public:
    UIManager(sf::RenderWindow& w, sf::Font& f, Graph& gr) : win(w), font(f), g(gr) {
        buildButtons();
    }

    void handleEvent(const sf::Event& ev, bool clicked, sf::Vector2i mp) {
        for (auto& b : algoBtns) b.update(mp, clicked);
        btnPrev.update(mp, clicked);
        btnNext.update(mp, clicked);
        btnReset.update(mp, clicked);
        btnAutoPlay.update(mp, clicked);
        btnAddEdge.update(mp, clicked);
        btnRemoveEdge.update(mp, clicked);

        if (ev.type == sf::Event::KeyPressed) {
            switch (ev.key.code) {
                case sf::Keyboard::N: case sf::Keyboard::Right: g.nextStep();        break;
                case sf::Keyboard::P: case sf::Keyboard::Left:  g.prevStep();        break;
                case sf::Keyboard::R:     g.reset(); autoPlay=false;                 break;
                case sf::Keyboard::Space: runAlgorithm();                            break;
                case sf::Keyboard::A:     autoPlay = !autoPlay;                      break;
                case sf::Keyboard::Up:    selAlgo = (selAlgo+1)%6;                  break;
                case sf::Keyboard::Down:  selAlgo = (selAlgo+5)%6;                  break;
                default: break;
            }
        }

        if (clicked) {
            float bx = 870;
            float sy = 30 + 6*44 + 20 + 200;

            int dS = spinnerClick(mp, bx, sy);
            if (dS != 0) startNode = (startNode + dS + g.V) % g.V;

            int dU = spinnerClick(mp, bx, sy+22);
            if (dU != 0) edgeU = (edgeU + dU + g.V) % g.V;

            int dV = spinnerClick(mp, bx, sy+44);
            if (dV != 0) edgeV = (edgeV + dV + g.V) % g.V;

            int dW = spinnerClick(mp, bx, sy+66);
            if (dW != 0) edgeW = max(1, edgeW + dW);

            int dSp = spinnerClick(mp, bx, sy+88);
            if (dSp != 0) autoSpeed = max(0.5f, min(5.f, autoSpeed + 0.5f*dSp));
        }
    }

    void update(float dt) {
        if (autoPlay) {
            autoTimer += dt;
            if (autoTimer >= 1.f/autoSpeed) {
                autoTimer = 0.f;
                if (g.currentStep < (int)g.steps.size()-1) g.nextStep();
                else autoPlay = false;
            }
        }
    }

    void draw() {
        float bx = 870, bw = 190;

        drawPanel(win, {bx-10, 0}, {bw+20, 700});

        drawText("GRAPH", bx+2, 10, 22, Palette::ACCENT, true);
        drawText("VISUALIZER", bx, 32, 13, Palette::ACCENT2);
        drawSeparator(bx-5, 58, bw+10);

        float by = 65, bh = 34, gap = 8;
        drawText("ALGORITHMS", bx, by-18, 11, Palette::TEXT_MUTED);

        for (int i = 0; i < (int)algoBtns.size(); ++i) {
            if (i == selAlgo) {
                sf::RectangleShape sel({bw, bh});
                sel.setPosition(bx, by + i*(bh+gap));
                sel.setFillColor(sf::Color(40,50,100));
                sel.setOutlineThickness(1.5f);
                sel.setOutlineColor(Palette::ACCENT);
                win.draw(sel);
                sf::Text t; t.setFont(font); t.setString(algoNames[i]);
                t.setCharacterSize(13); t.setStyle(sf::Text::Bold);
                t.setFillColor(Palette::ACCENT);
                sf::FloatRect tb = t.getLocalBounds();
                t.setOrigin(tb.left+tb.width/2, tb.top+tb.height/2);
                t.setPosition(bx+bw/2, by+i*(bh+gap)+bh/2);
                win.draw(t);
            } else {
                algoBtns[i].draw(win);
            }
        }

        float cy = by + 6*(bh+gap) + 8;
        drawSeparator(bx-5, cy, bw+10);
        cy += 8;
        drawText("PLAYBACK", bx, cy, 11, Palette::TEXT_MUTED);
        cy += 16;

        btnPrev.draw(win); btnNext.draw(win);
        btnReset.draw(win);
        btnAutoPlay.rect.setFillColor(autoPlay ? Palette::SUCCESS : sf::Color(20,50,30));
        btnAutoPlay.label.setString(autoPlay ? "Pause" : "Auto");
        btnAutoPlay.draw(win);

        cy += 90;
        drawSeparator(bx-5, cy, bw+10);
        cy += 8;
        drawText("PARAMETERS", bx, cy, 11, Palette::TEXT_MUTED);
        cy += 18;

        drawSpinner(bx, cy,    (float)startNode, 0, g.V-1, "Start",  Palette::WARNING);
        drawSpinner(bx, cy+22, (float)edgeU,     0, g.V-1, "U",      Palette::ACCENT);
        drawSpinner(bx, cy+44, (float)edgeV,     0, g.V-1, "V",      Palette::ACCENT);
        drawSpinner(bx, cy+66, (float)edgeW,     1, 99,    "Weight", Palette::SUCCESS);
        drawSpinner(bx, cy+88, autoSpeed,         .5f,5.f, "Speed",  Palette::ACCENT2);

        cy += 112;
        btnAddEdge.draw(win);
        btnRemoveEdge.draw(win);

        float ix = 10, iy = 555, iw = 840, ih = 135;
        drawPanel(win, {ix, iy}, {iw, ih});
        drawText("STEP LOG", ix+10, iy+8, 11, Palette::TEXT_MUTED);

        int cur = g.currentStep;
        int tot = (int)g.steps.size();

        float pct = tot > 1 ? (float)cur/(tot-1) : 0.f;
        sf::RectangleShape pbg({iw-20, 4});
        pbg.setPosition(ix+10, iy+24); pbg.setFillColor(sf::Color(40,48,70));
        win.draw(pbg);
        sf::RectangleShape pfg({(iw-20)*pct, 4});
        pfg.setPosition(ix+10, iy+24); pfg.setFillColor(Palette::ACCENT);
        win.draw(pfg);

        drawText("Step " + to_string(cur+1) + " / " + to_string(max(1,tot)), ix+10, iy+34, 12, Palette::ACCENT);

        if (!g.steps.empty() && cur < tot)
            drawText(g.steps[cur], ix+10, iy+54, 14, Palette::TEXT_PRIMARY, true);

        int shown = 0;
        for (int i = cur-1; i >= max(0, cur-3); --i) {
            sf::Color c = Palette::TEXT_MUTED; c.a = (sf::Uint8)(180.f - shown*50.f);
            drawText(g.steps[i], ix+10, iy+76 + shown*18, 12, c);
            ++shown;
        }

        float lx = 10, ly = 10;
        drawPanel(win, {lx, ly}, {155, 100});
        drawText("LEGEND", lx+8, ly+6, 11, Palette::TEXT_MUTED);

        auto dot = [&](float x, float y, sf::Color c, const string& lbl) {
            sf::CircleShape ci(6); ci.setOrigin(6,6);
            ci.setPosition(x+8, y+8); ci.setFillColor(c);
            win.draw(ci);
            drawText(lbl, x+20, y, 12, Palette::TEXT_MUTED);
        };
        dot(lx+5, ly+22, Palette::NODE_DEFAULT, "Unvisited");
        dot(lx+5, ly+40, Palette::NODE_QUEUED,  "Queued");
        dot(lx+5, ly+58, Palette::NODE_VISITED, "Visited");
        dot(lx+5, ly+76, Palette::NODE_ACTIVE,  "Active");
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1080, 700), "Graph Algorithm Visualizer",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    const vector<string> fontPaths = {
        "Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/Arial.ttf"
    };
    bool fontLoaded = false;
    for (auto& fp : fontPaths) {
        if (font.loadFromFile(fp)) { fontLoaded = true; break; }
    }
    if (!fontLoaded)
        cerr << "Warning: Could not load font.\n";

    Graph graph(7, false);
    graph.addEdge(0, 1, 4);
    graph.addEdge(0, 2, 2);
    graph.addEdge(1, 2, 1);
    graph.addEdge(1, 3, 5);
    graph.addEdge(2, 3, 8);
    graph.addEdge(2, 4, 10);
    graph.addEdge(3, 4, 2);
    graph.addEdge(3, 5, 6);
    graph.addEdge(4, 5, 3);
    graph.addEdge(5, 6, 7);
    graph.reset();

    GraphRenderer renderer(window, font, graph);
    UIManager ui(window, font, graph);

    sf::Clock clock;

    while (window.isOpen()) {
        bool clicked = false;
        sf::Vector2i mp = sf::Mouse::getPosition(window);

        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left) clicked = true;
            ui.handleEvent(ev, clicked, mp);
        }

        ui.handleEvent(sf::Event{}, false, mp);

        float dt = clock.restart().asSeconds();
        ui.update(dt);

        window.clear(Palette::BG);
        renderer.draw();
        ui.draw();
        window.display();
    }

    return 0;
}