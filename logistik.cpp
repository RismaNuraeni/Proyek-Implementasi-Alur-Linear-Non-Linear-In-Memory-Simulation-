/*
 * ============================================================
 *  SISTEM MANAJEMEN & ROUTING PAKET LOGISTIK
 *  In-Memory Simulator — Struktur Data
 *
 *  Struktur Data yang Digunakan:
 *    1. Queue (FIFO)         — Gudang/Hub setiap kota
 *    2. Directed Graph       — Peta rute (Adjacency List)
 *    3. Linked List          — Histori perjalanan paket (resi)
 * ============================================================
 */

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
using namespace std;

// ============================================================
//  SECTION 1: LINKED LIST — Histori Resi Paket
// ============================================================

struct LogNode {
    string kota;       // Nama kota/hub yang disinggahi
    string timestamp;  // Waktu pencatatan log
    string status;     // Status: "DITERIMA", "TRANSIT", "TIBA"
    LogNode* next;

    LogNode(const string& k, const string& t, const string& s)
        : kota(k), timestamp(t), status(s), next(nullptr) {}
};

struct HistoriResi {
    LogNode* head;
    LogNode* tail;
    int jumlahLog;

    HistoriResi() : head(nullptr), tail(nullptr), jumlahLog(0) {}

    // Insert di tail — O(1) karena menyimpan pointer tail
    void tambahLog(const string& kota, const string& status) {
        time_t now = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

        LogNode* node = new LogNode(kota, string(buf), status);
        if (tail == nullptr) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        jumlahLog++;
    }

    // Traversal dari head ke tail — O(n)
    void tampilkan() const {
        if (head == nullptr) {
            cout << "  [Belum ada histori perjalanan]\n";
            return;
        }
        LogNode* cur = head;
        int urutan = 1;
        while (cur != nullptr) {
            cout << "  " << urutan++ << ". [" << cur->timestamp << "]  "
                 << left << setw(14) << cur->kota << "  --> " << cur->status << "\n";
            cur = cur->next;
        }
    }

    ~HistoriResi() {
        LogNode* cur = head;
        while (cur) {
            LogNode* tmp = cur->next;
            delete cur;
            cur = tmp;
        }
    }
};

// ============================================================
//  SECTION 2: STRUKTUR PAKET
// ============================================================

struct Paket {
    string noResi;
    string kotaAsal;
    string kotaTujuan;
    string kotaSekarang;
    string pengirim;
    string penerima;
    bool sudahSampai;
    HistoriResi* histori;
    Paket* nextInQueue;

    Paket(const string& resi, const string& asal, const string& tujuan,
          const string& pen, const string& per)
        : noResi(resi), kotaAsal(asal), kotaTujuan(tujuan),
          kotaSekarang(asal), pengirim(pen), penerima(per),
          sudahSampai(false), nextInQueue(nullptr) {
        histori = new HistoriResi();
    }

    ~Paket() { delete histori; }
};

// ============================================================
//  SECTION 3: QUEUE (FIFO) — Hub/Gudang Per Kota
// ============================================================

struct HubQueue {
    string namaKota;
    Paket* head;
    Paket* tail;
    int jumlah;
    HubQueue* nextHub;

    HubQueue(const string& nama)
        : namaKota(nama), head(nullptr), tail(nullptr), jumlah(0), nextHub(nullptr) {}

    // Enqueue di belakang — O(1)
    void enqueue(Paket* p) {
        p->nextInQueue = nullptr;
        if (tail == nullptr) {
            head = tail = p;
        } else {
            tail->nextInQueue = p;
            tail = p;
        }
        jumlah++;
    }

    // Dequeue dari depan — O(1)
    Paket* dequeue() {
        if (head == nullptr) return nullptr;
        Paket* p = head;
        head = head->nextInQueue;
        if (head == nullptr) tail = nullptr;
        p->nextInQueue = nullptr;
        jumlah--;
        return p;
    }

    bool isEmpty() const { return head == nullptr; }
};

// ============================================================
//  SECTION 4: DIRECTED GRAPH — Peta Rute (Adjacency List)
// ============================================================

struct EdgeNode {
    string kotaTujuan;
    int bobot;
    EdgeNode* next;

    EdgeNode(const string& tujuan, int b)
        : kotaTujuan(tujuan), bobot(b), next(nullptr) {}
};

struct VertexNode {
    string namaKota;
    EdgeNode* daftarTetangga;
    VertexNode* next;

    VertexNode(const string& nama)
        : namaKota(nama), daftarTetangga(nullptr), next(nullptr) {}

    ~VertexNode() {
        EdgeNode* cur = daftarTetangga;
        while (cur) { EdgeNode* tmp = cur->next; delete cur; cur = tmp; }
    }
};

struct Graf {
    VertexNode* head;
    int jumlahKota;

    Graf() : head(nullptr), jumlahKota(0) {}

    VertexNode* cariVertex(const string& nama) const {
        VertexNode* cur = head;
        while (cur) { if (cur->namaKota == nama) return cur; cur = cur->next; }
        return nullptr;
    }

    void tambahKota(const string& nama) {
        if (cariVertex(nama) != nullptr) return;
        VertexNode* node = new VertexNode(nama);
        if (head == nullptr) { head = node; }
        else { VertexNode* cur = head; while (cur->next) cur = cur->next; cur->next = node; }
        jumlahKota++;
    }

    // Tambah edge berarah A -> B dengan bobot jarak
    void tambahJalur(const string& dari, const string& ke, int jarak) {
        if (cariVertex(dari) == nullptr) tambahKota(dari);
        if (cariVertex(ke)   == nullptr) tambahKota(ke);
        VertexNode* v = cariVertex(dari);
        EdgeNode* edge = new EdgeNode(ke, jarak);
        edge->next = v->daftarTetangga;
        v->daftarTetangga = edge;
    }

    // Cari tetangga terdekat (greedy next-hop, bobot terkecil) — O(E)
    string cariTetanggaTerdekat(const string& dariKota) const {
        VertexNode* v = cariVertex(dariKota);
        if (v == nullptr || v->daftarTetangga == nullptr) return "";
        EdgeNode* cur = v->daftarTetangga;
        string terbaik = cur->kotaTujuan;
        int jarakMin = cur->bobot;
        cur = cur->next;
        while (cur) {
            if (cur->bobot < jarakMin) { jarakMin = cur->bobot; terbaik = cur->kotaTujuan; }
            cur = cur->next;
        }
        return terbaik;
    }

    void tampilkanPeta() const {
        cout << "\n  === PETA RUTE DISTRIBUSI ===\n";
        VertexNode* v = head;
        while (v) {
            cout << "  " << v->namaKota << " :\n";
            EdgeNode* e = v->daftarTetangga;
            if (!e) cout << "    (tidak ada jalur keluar)\n";
            while (e) {
                cout << "    -> " << left << setw(14) << e->kotaTujuan
                     << "  " << e->bobot << " km\n";
                e = e->next;
            }
            v = v->next;
        }
    }

    ~Graf() {
        VertexNode* cur = head;
        while (cur) { VertexNode* tmp = cur->next; delete cur; cur = tmp; }
    }
};

// ============================================================
//  SECTION 5: SISTEM UTAMA
// ============================================================

struct Sistem {
    Graf* peta;
    HubQueue* daftarHub;
    int counterResi;

    Sistem() : daftarHub(nullptr), counterResi(1000) {
        peta = new Graf();
        inisialisasiPetaDefault();
    }

    void inisialisasiPetaDefault() {
        peta->tambahJalur("Jakarta",    "Bekasi",      30);
        peta->tambahJalur("Jakarta",    "Bogor",       60);
        peta->tambahJalur("Bekasi",     "Karawang",    45);
        peta->tambahJalur("Karawang",   "Purwakarta",  40);
        peta->tambahJalur("Purwakarta", "Bandung",     70);
        peta->tambahJalur("Bogor",      "Bandung",    120);
        peta->tambahJalur("Bandung",    "Sumedang",    45);
        peta->tambahJalur("Bandung",    "Garut",       65);
        peta->tambahJalur("Sumedang",   "Majalengka",  40);
        peta->tambahJalur("Majalengka", "Cirebon",     55);
        peta->tambahJalur("Garut",      "Tasikmalaya", 70);
        peta->tambahJalur("Tasikmalaya","Ciamis",      35);
        peta->tambahJalur("Ciamis",     "Banjar",      25);

        const string kota[] = {
            "Jakarta","Bekasi","Bogor","Karawang","Purwakarta",
            "Bandung","Sumedang","Garut","Majalengka",
            "Cirebon","Tasikmalaya","Ciamis","Banjar"
        };
        for (const string& k : kota) tambahHub(k);
    }

    void tambahHub(const string& nama) {
        if (cariHub(nama) != nullptr) return;
        HubQueue* hub = new HubQueue(nama);
        if (daftarHub == nullptr) { daftarHub = hub; return; }
        HubQueue* cur = daftarHub;
        while (cur->nextHub) cur = cur->nextHub;
        cur->nextHub = hub;
    }

    HubQueue* cariHub(const string& nama) const {
        HubQueue* cur = daftarHub;
        while (cur) { if (cur->namaKota == nama) return cur; cur = cur->nextHub; }
        return nullptr;
    }

    void registrasiPaket(const string& asal, const string& tujuan,
                          const string& pengirim, const string& penerima) {
        if (peta->cariVertex(asal) == nullptr) {
            cout << "  [ERROR] Kota asal '" << asal << "' tidak ada di peta!\n"; return;
        }
        if (peta->cariVertex(tujuan) == nullptr) {
            cout << "  [ERROR] Kota tujuan '" << tujuan << "' tidak ada di peta!\n"; return;
        }
        string resi = "LOG-" + to_string(++counterResi);
        Paket* p = new Paket(resi, asal, tujuan, pengirim, penerima);
        p->histori->tambahLog(asal, "DITERIMA DI HUB ASAL");
        cariHub(asal)->enqueue(p);

        cout << "\n  Paket berhasil didaftarkan!\n";
        cout << "    No. Resi  : " << resi << "\n";
        cout << "    Pengirim  : " << pengirim << "\n";
        cout << "    Penerima  : " << penerima << "\n";
        cout << "    Rute      : " << asal << "  -->  " << tujuan << "\n";
        cout << "    Status    : Menunggu di antrian Hub " << asal << "\n";
    }

    void prosesPengiriman(const string& namaHub) {
        HubQueue* hub = cariHub(namaHub);
        if (hub == nullptr || hub->isEmpty()) {
            cout << "  [INFO] Tidak ada paket di antrian Hub " << namaHub << ".\n"; return;
        }
        Paket* p = hub->dequeue();
        cout << "\n  >> Memproses paket " << p->noResi << " dari Hub " << namaHub << "...\n";

        if (p->kotaSekarang == p->kotaTujuan || p->sudahSampai) {
            p->sudahSampai = true;
            p->histori->tambahLog(p->kotaTujuan, "PAKET TIBA DI TUJUAN AKHIR");
            cout << "  Paket sudah tiba di tujuan: " << p->kotaTujuan << "\n";
            hub->enqueue(p);
            return;
        }

        string kotaBerikutnya = peta->cariTetanggaTerdekat(p->kotaSekarang);
        if (kotaBerikutnya.empty()) {
            cout << "  [ERROR] Tidak ada jalur dari " << p->kotaSekarang << "!\n";
            hub->enqueue(p); return;
        }

        p->kotaSekarang = kotaBerikutnya;
        if (kotaBerikutnya == p->kotaTujuan) {
            p->sudahSampai = true;
            p->histori->tambahLog(kotaBerikutnya, "TIBA DI TUJUAN");
        } else {
            p->histori->tambahLog(kotaBerikutnya, "TRANSIT");
        }

        tambahHub(kotaBerikutnya);
        cariHub(kotaBerikutnya)->enqueue(p);
        cout << "  Paket diteruskan ke Hub " << kotaBerikutnya << ".\n";
        cout << "    Posisi kini : " << p->kotaSekarang << "\n";
        cout << "    Status      : " << (p->sudahSampai ? "TIBA DI TUJUAN" : "TRANSIT") << "\n";
    }

    void lacakResi(const string& noResi) const {
        HubQueue* hub = daftarHub;
        while (hub) {
            Paket* cur = hub->head;
            while (cur) {
                if (cur->noResi == noResi) {
                    cout << "\n  ┌─────────────────────────────────────┐\n";
                    cout << "  │         HASIL LACAK RESI             │\n";
                    cout << "  └─────────────────────────────────────┘\n";
                    cout << "  No. Resi    : " << cur->noResi << "\n";
                    cout << "  Pengirim    : " << cur->pengirim << "\n";
                    cout << "  Penerima    : " << cur->penerima << "\n";
                    cout << "  Asal        : " << cur->kotaAsal << "\n";
                    cout << "  Tujuan      : " << cur->kotaTujuan << "\n";
                    cout << "  Posisi Kini : " << cur->kotaSekarang << "\n";
                    cout << "  Status      : " << (cur->sudahSampai ? "TERKIRIM" : "DALAM PERJALANAN") << "\n";
                    cout << "\n  --- Histori Perjalanan ---\n";
                    cur->histori->tampilkan();
                    return;
                }
                cur = cur->nextInQueue;
            }
            hub = hub->nextHub;
        }
        cout << "  [INFO] Resi '" << noResi << "' tidak ditemukan.\n";
    }

    void tampilkanStatusHub() const {
        cout << "\n  === STATUS ANTRIAN HUB ===\n";
        bool adaPaket = false;
        HubQueue* hub = daftarHub;
        while (hub) {
            if (hub->jumlah > 0) {
                adaPaket = true;
                cout << "  Hub " << left << setw(15) << hub->namaKota
                     << " | " << hub->jumlah << " paket"
                     << " | Depan: " << hub->head->noResi << "\n";
            }
            hub = hub->nextHub;
        }
        if (!adaPaket) cout << "  (Semua hub kosong)\n";
    }

    void tampilkanDaftarKota() const {
        cout << "\n  Kota tersedia : ";
        VertexNode* v = peta->head;
        while (v) {
            cout << v->namaKota;
            if (v->next) cout << ", ";
            v = v->next;
        }
        cout << "\n";
    }

    ~Sistem() {
        HubQueue* hub = daftarHub;
        while (hub) {
            while (!hub->isEmpty()) { Paket* p = hub->dequeue(); delete p; }
            HubQueue* tmp = hub->nextHub;
            delete hub;
            hub = tmp;
        }
        delete peta;
    }
};

// ============================================================
//  SECTION 6: ANTARMUKA CLI
// ============================================================

void tampilkanHeader() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════╗\n";
    cout << "  ║    SISTEM MANAJEMEN & ROUTING PAKET LOGISTIK     ║\n";
    cout << "  ║       In-Memory Simulator  |  Struktur Data      ║\n";
    cout << "  ╚══════════════════════════════════════════════════╝\n";
}

void tampilkanMenu() {
    cout << "\n  ┌─────────────────────────────────────┐\n";
    cout << "  │              MENU UTAMA              │\n";
    cout << "  ├─────────────────────────────────────┤\n";
    cout << "  │  1. Registrasi Paket Baru            │\n";
    cout << "  │  2. Proses Pengiriman (Dispatch)     │\n";
    cout << "  │  3. Lacak Resi Paket                 │\n";
    cout << "  │  4. Lihat Status Semua Hub           │\n";
    cout << "  │  5. Lihat Peta Rute Distribusi       │\n";
    cout << "  │  6. Lihat Daftar Kota                │\n";
    cout << "  │  0. Keluar                           │\n";
    cout << "  └─────────────────────────────────────┘\n";
    cout << "  Pilihan: ";
}

string inputBaris(const string& prompt) {
    cout << "  " << prompt;
    string s;
    getline(cin, s);
    return s;
}

int main() {
    Sistem sistem;
    tampilkanHeader();
    cout << "\n  Sistem dimuat. Peta logistik Jawa Barat telah diinisialisasi.\n";

    int pilihan;
    string input;

    do {
        tampilkanMenu();
        getline(cin, input);
        if (input.empty()) { pilihan = -1; continue; }
        try { pilihan = stoi(input); } catch (...) { pilihan = -1; }

        switch (pilihan) {
            case 1: {
                cout << "\n  -- REGISTRASI PAKET BARU --\n";
                sistem.tampilkanDaftarKota();
                string asal     = inputBaris("Kota Asal     : ");
                string tujuan   = inputBaris("Kota Tujuan   : ");
                string pengirim = inputBaris("Nama Pengirim : ");
                string penerima = inputBaris("Nama Penerima : ");
                sistem.registrasiPaket(asal, tujuan, pengirim, penerima);
                break;
            }
            case 2: {
                cout << "\n  -- PROSES PENGIRIMAN (DISPATCH) --\n";
                sistem.tampilkanStatusHub();
                string namaHub = inputBaris("\nNama Hub yang diproses: ");
                sistem.prosesPengiriman(namaHub);
                break;
            }
            case 3: {
                cout << "\n  -- LACAK RESI PAKET --\n";
                string resi = inputBaris("Masukkan No. Resi: ");
                sistem.lacakResi(resi);
                break;
            }
            case 4: sistem.tampilkanStatusHub(); break;
            case 5: sistem.peta->tampilkanPeta(); break;
            case 6: sistem.tampilkanDaftarKota(); break;
            case 0: cout << "\n  Terima kasih. Program dihentikan.\n\n"; break;
            default: cout << "  [!] Pilihan tidak valid.\n";
        }
    } while (pilihan != 0);

    return 0;
}

