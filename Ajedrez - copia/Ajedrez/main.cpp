#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <locale>
#include <cwchar>
#include <chrono>
#include <thread>
#include <iomanip>
#include <limits> // Para std::numeric_limits

// =========================================================================
// 1. ENUMERACIONES
// =========================================================================

enum class ColorPieza {
    BLANCO,
    NEGRO,
    NINGUNO
};

enum class TipoPieza {
    PEON,
    TORRE,
    CABALLO,
    ALFIL,
    REINA,
    REY,
    VACIO
};

// =========================================================================
// 2. CLASE PIEZA (CON CARACTERES ASCII SIMPLES)
// =========================================================================

class Pieza {
public:
    ColorPieza color;
    TipoPieza tipo;
    std::string simbolo;

    Pieza(ColorPieza c = ColorPieza::NINGUNO, TipoPieza t = TipoPieza::VACIO) : color(c), tipo(t) {
        if (t == TipoPieza::VACIO) {
            simbolo = " . ";
        }
        else if (c == ColorPieza::BLANCO) {
            switch (t) {
            case TipoPieza::PEON:   simbolo = " P "; break;
            case TipoPieza::TORRE:  simbolo = " T "; break;
            case TipoPieza::CABALLO: simbolo = " C "; break;
            case TipoPieza::ALFIL:  simbolo = " A "; break;
            case TipoPieza::REINA:  simbolo = " D "; break;
            case TipoPieza::REY:    simbolo = " R "; break;
            default: simbolo = " ? "; break;
            }
        }
        else { // ColorPieza::NEGRO
            switch (t) {
            case TipoPieza::PEON:   simbolo = " p "; break;
            case TipoPieza::TORRE:  simbolo = " t "; break;
            case TipoPieza::CABALLO: simbolo = " c "; break;
            case TipoPieza::ALFIL:  simbolo = " a "; break;
            case TipoPieza::REINA:  simbolo = " d "; break;
            case TipoPieza::REY:    simbolo = " r "; break;
            default: simbolo = " ? "; break;
            }
        }
    }
};

// =========================================================================
// 3. FUNCIONES AUXILIARES (Coordenadas y Tiempo)
// =========================================================================

bool coordenadasAIndices(const std::string& coord, int& fila, int& col) {
    if (coord.length() != 2) return false;
    char c = coord[0];
    char f = coord[1];
    if (c >= 'a' && c <= 'h' && f >= '1' && f <= '8') {
        col = c - 'a';
        fila = '8' - f;
        return true;
    }
    return false;
}

void mostrarTiempo(const std::string& label, std::chrono::seconds restante) {
    auto min = std::chrono::duration_cast<std::chrono::minutes>(restante);
    auto seg = std::chrono::duration_cast<std::chrono::seconds>(restante - min);

    std::cout << label << ": ["
        << std::setfill('0') << std::setw(2) << min.count() << ":"
        << std::setfill('0') << std::setw(2) << seg.count()
        << "]";
}

// =========================================================================
// 4. CLASE TABLERO (Declaración e Implementación)
// =========================================================================

class Tablero {
public:
    std::vector<std::vector<Pieza>> tablero;
    ColorPieza turnoActual;
    Tablero() : tablero(8, std::vector<Pieza>(8)), turnoActual(ColorPieza::BLANCO) { inicializarTablero(); }
    void inicializarTablero();
    void imprimirTablero() const;
    bool moverPieza(int filaOrigen, int colOrigen, int filaDest, int colDest);
    bool estaEnJaque(ColorPieza color) const;
    bool hayMovimientosLegales(ColorPieza color);
private:
    void colocarPieza(int fila, int col, ColorPieza color, TipoPieza tipo);
    bool esMovimientoLegal(int filaOrigen, int colOrigen, int filaDest, int colDest) const;
    bool buscarRey(ColorPieza color, int& fr, int& cr) const;
    bool simularYMover(int fo, int co, int fd, int cd);
};

void Tablero::colocarPieza(int fila, int col, ColorPieza color, TipoPieza tipo) { tablero[fila][col] = Pieza(color, tipo); }
void Tablero::inicializarTablero() {
    for (int i = 0; i < 8; ++i) { for (int j = 0; j < 8; ++j) { colocarPieza(i, j, ColorPieza::NINGUNO, TipoPieza::VACIO); } }
    for (int j = 0; j < 8; ++j) { colocarPieza(1, j, ColorPieza::NEGRO, TipoPieza::PEON); colocarPieza(6, j, ColorPieza::BLANCO, TipoPieza::PEON); }
    colocarPieza(0, 0, ColorPieza::NEGRO, TipoPieza::TORRE); colocarPieza(0, 7, ColorPieza::NEGRO, TipoPieza::TORRE);
    colocarPieza(0, 1, ColorPieza::NEGRO, TipoPieza::CABALLO); colocarPieza(0, 6, ColorPieza::NEGRO, TipoPieza::CABALLO);
    colocarPieza(0, 2, ColorPieza::NEGRO, TipoPieza::ALFIL); colocarPieza(0, 5, ColorPieza::NEGRO, TipoPieza::ALFIL);
    colocarPieza(0, 3, ColorPieza::NEGRO, TipoPieza::REINA); colocarPieza(0, 4, ColorPieza::NEGRO, TipoPieza::REY);
    colocarPieza(7, 0, ColorPieza::BLANCO, TipoPieza::TORRE); colocarPieza(7, 7, ColorPieza::BLANCO, TipoPieza::TORRE);
    colocarPieza(7, 1, ColorPieza::BLANCO, TipoPieza::CABALLO); colocarPieza(7, 6, ColorPieza::BLANCO, TipoPieza::CABALLO);
    colocarPieza(7, 2, ColorPieza::BLANCO, TipoPieza::ALFIL); colocarPieza(7, 5, ColorPieza::BLANCO, TipoPieza::ALFIL);
    colocarPieza(7, 3, ColorPieza::BLANCO, TipoPieza::REINA); colocarPieza(7, 4, ColorPieza::BLANCO, TipoPieza::REY);
}
void Tablero::imprimirTablero() const {
    std::cout << "\n      a  b  c  d  e  f  g  h\n";
    std::cout << "    ---------------------------\n";
    for (int i = 0; i < 8; ++i) {
        std::cout << " " << 8 - i << " |";
        for (int j = 0; j < 8; ++j) {
            std::cout << tablero[i][j].simbolo;
        }
        std::cout << "| " << 8 - i << "\n";
    }
    std::cout << "    ---------------------------\n";
    std::cout << "      a  b  c  d  e  f  g  h\n";
}
// [Funciones esMovimientoLegal, buscarRey, estaEnJaque, simularYMover, hayMovimientosLegales, moverPieza son las mismas]
bool Tablero::esMovimientoLegal(int filaOrigen, int colOrigen, int filaDest, int colDest) const {
    const Pieza& piezaOrigen = tablero[filaOrigen][colOrigen];
    const Pieza& piezaDestino = tablero[filaDest][colDest];
    if (piezaDestino.color == piezaOrigen.color && piezaDestino.tipo != TipoPieza::VACIO) return false;
    int df = filaDest - filaOrigen; int dc = colDest - colOrigen; using std::abs;
    switch (piezaOrigen.tipo) {
    case TipoPieza::PEON: {
        int dir = (piezaOrigen.color == ColorPieza::BLANCO) ? -1 : 1;
        if (dc == 0 && df == dir && piezaDestino.tipo == TipoPieza::VACIO) return true;
        bool esPrimeraFila = (piezaOrigen.color == ColorPieza::BLANCO && filaOrigen == 6) || (piezaOrigen.color == ColorPieza::NEGRO && filaOrigen == 1);
        if (esPrimeraFila && dc == 0 && df == 2 * dir && piezaDestino.tipo == TipoPieza::VACIO && tablero[filaOrigen + dir][colOrigen].tipo == TipoPieza::VACIO) return true;
        if (std::abs(dc) == 1 && df == dir && piezaDestino.tipo != TipoPieza::VACIO) return true; return false;
    }
    case TipoPieza::TORRE: {
        if (df != 0 && dc != 0) return false;
        if (df == 0) { int step = (dc > 0) ? 1 : -1; for (int c = colOrigen + step; c != colDest; c += step) { if (tablero[filaOrigen][c].tipo != TipoPieza::VACIO) return false; } }
        else { int step = (df > 0) ? 1 : -1; for (int f = filaOrigen + step; f != filaDest; f += step) { if (tablero[f][colOrigen].tipo != TipoPieza::VACIO) return false; } } return true;
    }
    case TipoPieza::CABALLO: { return (std::abs(df) == 2 && std::abs(dc) == 1) || (std::abs(df) == 1 && std::abs(dc) == 2); }
    case TipoPieza::ALFIL: {
        if (std::abs(df) != std::abs(dc) || df == 0) return false;
        int step_f = (df > 0) ? 1 : -1; int step_c = (dc > 0) ? 1 : -1; int f = filaOrigen + step_f; int c = colOrigen + step_c;
        while (f != filaDest) { if (tablero[f][c].tipo != TipoPieza::VACIO) return false; f += step_f; c += step_c; } return true;
    }
    case TipoPieza::REINA: {
        bool esRecto = (df == 0 || dc == 0); bool esDiagonal = (std::abs(df) == std::abs(dc) && df != 0); if (!esRecto && !esDiagonal) return false;
        if (esRecto) {
            if (df == 0) { int step = (dc > 0) ? 1 : -1; for (int c = colOrigen + step; c != colDest; c += step) { if (tablero[filaOrigen][c].tipo != TipoPieza::VACIO) return false; } }
            else { int step = (df > 0) ? 1 : -1; for (int f = filaOrigen + step; f != filaDest; f += step) { if (tablero[f][colOrigen].tipo != TipoPieza::VACIO) return false; } }
        }
        else {
            int step_f = (df > 0) ? 1 : -1; int step_c = (dc > 0) ? 1 : -1; int f = filaOrigen + step_f; int c = colOrigen + step_c;
            while (f != filaDest) { if (tablero[f][c].tipo != TipoPieza::VACIO) return false; f += step_f; c += step_c; }
        } return true;
    }
    case TipoPieza::REY: { return std::abs(df) <= 1 && std::abs(dc) <= 1 && (df != 0 || dc != 0); }
    default: return false;
    }
}
bool Tablero::buscarRey(ColorPieza color, int& fr, int& cr) const {
    for (fr = 0; fr < 8; ++fr) { for (cr = 0; cr < 8; ++cr) { const Pieza& p = tablero[fr][cr]; if (p.tipo == TipoPieza::REY && p.color == color) { return true; } } } return false;
}
bool Tablero::estaEnJaque(ColorPieza color) const {
    int fr, cr; if (!buscarRey(color, fr, cr)) return false; ColorPieza colorOponente = (color == ColorPieza::BLANCO) ? ColorPieza::NEGRO : ColorPieza::BLANCO;
    for (int i = 0; i < 8; ++i) { for (int j = 0; j < 8; ++j) { const Pieza& pieza = tablero[i][j]; if (pieza.color == colorOponente) { if (esMovimientoLegal(i, j, fr, cr)) { return true; } } } } return false;
}
bool Tablero::simularYMover(int fo, int co, int fd, int cd) {
    Pieza piezaOrigen = tablero[fo][co]; Pieza piezaDestinoOriginal = tablero[fd][cd]; ColorPieza colorJugador = piezaOrigen.color;
    tablero[fd][cd] = piezaOrigen; tablero[fo][co] = Pieza(ColorPieza::NINGUNO, TipoPieza::VACIO);
    bool quedaEnJaque = estaEnJaque(colorJugador);
    tablero[fo][co] = piezaOrigen; tablero[fd][cd] = piezaDestinoOriginal;
    return !quedaEnJaque;
}
bool Tablero::hayMovimientosLegales(ColorPieza color) {
    for (int fo = 0; fo < 8; ++fo) {
        for (int co = 0; co < 8; ++co) {
            const Pieza& p = tablero[fo][co]; if (p.color == color) {
                for (int fd = 0; fd < 8; ++fd) { for (int cd = 0; cd < 8; ++cd) { if (esMovimientoLegal(fo, co, fd, cd)) { if (simularYMover(fo, co, fd, cd)) { return true; } } } }
            }
        }
    } return false;
}
bool Tablero::moverPieza(int filaOrigen, int colOrigen, int filaDest, int colDest) {
    const Pieza& piezaOrigen = tablero[filaOrigen][colOrigen];
    if (filaOrigen < 0 || filaOrigen >= 8 || colOrigen < 0 || colOrigen >= 8 || filaDest < 0 || filaDest >= 8 || colDest < 0 || colDest >= 8) { std::cout << "ERROR: Movimiento fuera de los límites del tablero.\n"; return false; }
    if (piezaOrigen.tipo == TipoPieza::VACIO) { std::cout << "ERROR: La casilla de origen está vacía.\n"; return false; }
    if (piezaOrigen.color != turnoActual) { std::cout << "ERROR: No es el turno de esta pieza.\n"; return false; }
    if (!esMovimientoLegal(filaOrigen, colOrigen, filaDest, colDest)) { std::cout << "ERROR: Movimiento no válido para esta pieza o está bloqueado.\n"; return false; }
    if (!simularYMover(filaOrigen, colOrigen, filaDest, colDest)) { std::cout << "ERROR: El movimiento deja/mantiene a tu Rey en jaque.\n"; return false; }
    tablero[filaDest][colDest] = piezaOrigen; tablero[filaOrigen][colOrigen] = Pieza(ColorPieza::NINGUNO, TipoPieza::VACIO);
    std::cout << "Movimiento realizado con éxito.\n";
    turnoActual = (turnoActual == ColorPieza::BLANCO) ? ColorPieza::NEGRO : ColorPieza::BLANCO;
    return true;
}


// =========================================================================
// 5. MAIN (Control del Juego con Cronómetro en Tiempo Real)
// =========================================================================

int main() {
    std::setlocale(LC_ALL, "");
    std::wcout.imbue(std::locale(""));

    Tablero tablero;
    std::string comando;
    bool juegoActivo = true;

    const std::chrono::minutes duracionInicial(10);
    std::chrono::seconds cronoBlanco = std::chrono::duration_cast<std::chrono::seconds>(duracionInicial);
    std::chrono::seconds cronoNegro = std::chrono::duration_cast<std::chrono::seconds>(duracionInicial);

    std::cout << "--- AJEDREZ C++ CON CRONÓMETRO (Tiempo Real) ---\n";
    std::cout << "El tiempo corre! Formato: e2e4\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    while (cronoBlanco.count() > 0 && cronoNegro.count() > 0 && juegoActivo) {

        ColorPieza siguienteTurno = tablero.turnoActual;
        std::string nombreTurno = (siguienteTurno == ColorPieza::BLANCO) ? "BLANCAS (P)" : "NEGRAS (p)";

        bool jaque = tablero.estaEnJaque(siguienteTurno);
        bool hayMovs = tablero.hayMovimientosLegales(siguienteTurno);

        if (!hayMovs) {
            juegoActivo = false;
            break;
        }

        bool movimientoRealizado = false;

        while (!movimientoRealizado && cronoBlanco.count() > 0 && cronoNegro.count() > 0) {

            // ⚠️ CORRECCIÓN: Usar "cls" para Windows
            system("cls"); // Cambiar a system("clear") para Linux/macOS

            tablero.imprimirTablero();

            std::cout << "Turno: " << nombreTurno;
            if (jaque) {
                std::wcout << L" ¡¡JAQUE!!";
            }
            std::cout << "\n";

            mostrarTiempo("BLANCAS", cronoBlanco);
            std::cout << "     ";
            mostrarTiempo("NEGRAS", cronoNegro);
            std::cout << "\n";

            std::cout << "Mover (ej: e2e4) o 'salir': ";
            std::cout.flush();

            // Pausa de 1 segundo (el corazón del cronómetro)
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Descontar 1 segundo
            if (siguienteTurno == ColorPieza::BLANCO) {
                cronoBlanco -= std::chrono::seconds(1);
            }
            else {
                cronoNegro -= std::chrono::seconds(1);
            }

            // Lógica de entrada
            std::string temp_comando;
            // Intenta leer la entrada. Si el usuario presiona ENTER, lee. Si no, continúa.
            // Esto es un compromiso, pero es la forma más limpia de manejar el tiempo real con std::cin.
            std::getline(std::cin, temp_comando);

            if (temp_comando.empty()) continue;

            comando = temp_comando;

            if (comando == "salir") { juegoActivo = false; break; }

            comando.erase(std::remove_if(comando.begin(), comando.end(), ::isspace), comando.end());

            if (comando.length() != 4) {
                // Notificación de error: el tiempo sigue corriendo.
                continue;
            }

            std::string origenStr = comando.substr(0, 2);
            std::string destinoStr = comando.substr(2, 2);
            int filaOrigen, colOrigen, filaDest, colDest;

            if (!coordenadasAIndices(origenStr, filaOrigen, colOrigen) ||
                !coordenadasAIndices(destinoStr, filaDest, colDest)) {
                // Notificación de error: el tiempo sigue corriendo.
                continue;
            }

            if (tablero.moverPieza(filaOrigen, colOrigen, filaDest, colDest)) {
                movimientoRealizado = true;
            }
        }

    }

    // --- Final del juego ---
    system("cls"); // Cambiar a system("clear") para Linux/macOS
    tablero.imprimirTablero();

    std::cout << "\n\n=== ¡Fin de la partida! ===\n";
    if (!juegoActivo && tablero.estaEnJaque(tablero.turnoActual)) {
        std::wcout << L"¡¡¡ JAQUE MATE !!! Ganaron las " << ((tablero.turnoActual == ColorPieza::BLANCO) ? L"NEGRAS" : L"BLANCAS") << L"!\n";
    }
    else if (!juegoActivo && !tablero.hayMovimientosLegales(tablero.turnoActual)) {
        std::wcout << L"¡¡¡ TABLAS POR AHOGADO !!!\n";
    }
    else if (cronoBlanco.count() <= 0) {
        std::wcout << L"¡Se ha acabado el tiempo de las BLANCAS! Ganan las NEGRAS.\n";
    }
    else if (cronoNegro.count() <= 0) {
        std::wcout << L"¡Se ha acabado el tiempo de las NEGRAS! Ganan las BLANCAS.\n";
    }
    else {
        std::wcout << L"Partida finalizada por el usuario.\n";
    }

    return 0;
}