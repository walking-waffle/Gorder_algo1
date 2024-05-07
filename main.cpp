#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Node {
    int id;
    int numOfDegree;
};

struct Edge {
    Node src;
    Node dst;
};

// 把值和index互換
vector<int> findIndexI( vector<int> originList ) {
    vector<int> invertedList( originList.size(), 0 );
    for ( int i = 0; i < originList.size(); i++ )
        invertedList.at(originList.at(i)) = i;

    return invertedList;
} // findIndexI

void init( string fileName ) {
    ifstream inputFile( fileName );
    if ( !inputFile ) {
        cerr << "Error: Unable to open input file." << endl;
        exit(1);
    } // if

    // konect 資料集中，%開頭需去除
    char temp;
    temp = inputFile.peek();
    while ( temp == '%' ) {
        inputFile.get(temp);
        while ( temp != '\n' )
            inputFile.get(temp);

        temp = inputFile.peek();
    } // while

    Node node1, node2;
    int startID;

    inputFile >> node1.id >> node2.id;

    vector<Edge> edgeList;
    edgeList.push_back({node1, node2});
    startID = min( node1.id, node2.id );

    // 讀取邊緣列表數據
    while ( inputFile >> node1.id >> node2.id ) {
        edgeList.push_back({node1, node2});
        startID = min( startID, node1.id );
        startID = min( startID, node2.id );
    } // while

    inputFile.close();

    if ( startID != 0 ) {
        for ( Edge & edge : edgeList ) {
            edge.src.id = edge.src.id - startID;
            edge.dst.id = edge.dst.id - startID;
        } // for
    } // if

    string name = fileName.substr( 0, fileName.find(".") );
    ofstream outputFile( name + ".txt" );
    for ( Edge edge : edgeList ) {
        outputFile << edge.src.id << " ";
        outputFile << edge.dst.id << "\n";
    } // for
    outputFile.close();
} // init

void readEdgeList( string fileName, vector<Edge> & edgeList, int & numOfNodes ) {
    ifstream inputFile( fileName );
    if ( !inputFile ) {
        cerr << "Error: Unable to open input file." << endl;
        exit(1);
    } // if

    Node node1, node2;

    // 讀取邊緣列表數據
    while ( inputFile >> node1.id >> node2.id ) {
        edgeList.push_back({node1, node2});
        numOfNodes = max( numOfNodes, node1.id );
        numOfNodes = max( numOfNodes, node2.id );
    } // while

    numOfNodes++;
    inputFile.close();
    cout << "N: " << numOfNodes << "\n";
    cout << "M: " << edgeList.size() << "\n";
} // readEdgeList

// 將圖的edge list格式轉換為CSR格式
void el2CSR( vector<Edge> edgeList, int numOfNodes, vector<int> & OA, vector<int> & EA ) {

    OA.resize( numOfNodes + 1, 0 );

    // 計算每個節點的鄰居數量
    for ( Edge edge : edgeList )
        OA.at(edge.src.id + 1)++;

    // 累積計算每個節點的起始位置
    for ( int i = 1; i <= numOfNodes; i++ )
        OA.at(i) += OA.at(i - 1);
    // ---------------------------------------- OA 完成
    EA.resize( edgeList.size() );

    // 將邊緣列表中的節點添加到對應的位置
    vector<int> nextIndex( numOfNodes, 0 );

    for ( Edge edge : edgeList ) {
        int node1 = edge.src.id;
        int node2 = edge.dst.id;
        int idx = OA.at(node1) + nextIndex.at(node1);
        EA.at(idx) = node2;
        nextIndex.at(node1)++;
    } // for
    // ---------------------------------------- EA 完成

} // el2CSR

void sortEdgeList( vector<Edge> & edgeList ) {
    sort(edgeList.begin(), edgeList.end(), [](const Edge & a, const Edge & b)->bool{
		if ( a.src.id < b.src.id )
			return true;
		else if ( a.src.id > b.src.id )
			return false;
		else {
			if( a.dst.id <= b.dst.id )
				return true;
			else
				return false;
		} // else
	});
} // sortEdgeList

// 輸入 edge list，輸出最大 in-degree 的 id
int findMaxInDegreeID( vector<Edge> edgeList, int numOfNodes ) {
    vector<int> inDegree ( numOfNodes, 0 );
    for ( Edge edge : edgeList )
        inDegree.at(edge.dst.id)++;

    auto maxIt = max_element( inDegree.begin(), inDegree.end() );
    return distance( inDegree.begin(), maxIt );
} // findMaxInDegreeID

int Fscore( int a, int b, vector<int> OA, vector<int> EA, vector<bool> visited ) {
    int score = 0;

    // a->b
    for ( int i = OA.at(a); i < OA.at(a+1); i++ ) {
        if ( EA.at(i) == b ) {
            score++;
            break;
        } // if
    } // for

    // a<-b
    for ( int i = OA.at(b); i < OA.at(b+1); i++ ) {
        if ( EA.at(i) == a ) {
            score++;
            break;
        } // if
    } // for

    for ( int i = 0; i < OA.size()-1; i++ ) {
        // 檢查節點 i 的鄰接點
        int start = OA.at(i);
        int end = OA.at(i + 1);

        bool iIsParentOfA = false, iIsParentOfB = false;
        // 遍歷 i 的所有鄰接點
        for ( int j = start; j < end; j++ ) {
            int neighbor = EA.at(j);

            if ( neighbor == a && visited.at(i) == false )
                iIsParentOfA = true;
            if ( neighbor == b && visited.at(i) == false )
                iIsParentOfB = true;
                
        } // for

        if ( iIsParentOfA && iIsParentOfB )
            score++;
    } // for

    return score;
} // Fscore

vector<Edge> Gorder( vector<Edge> edgeList, int numOfNodes ) {
    vector<int> OA, EA;
    el2CSR( edgeList, numOfNodes, OA, EA );

    vector<int> newOrder;
    vector<bool> visited ( numOfNodes, false );
    int begin = findMaxInDegreeID( edgeList, numOfNodes );
    cout << "begin: " << begin << endl;

    newOrder.push_back( begin );
    visited.at( begin ) = true;
    for ( int i = 1; i < numOfNodes; i++ ) {
        int vmax = 0, kmax = INT_MIN;

        for ( int v = 0; v < numOfNodes; v++ ) {
            int kv = 0;

            if ( visited.at(v) == false ) {
                for ( int j = max( 0, i-4 ); j < i; j++ ) 
                    kv = kv + Fscore( newOrder.at(j), v, OA, EA, visited );

                if ( kv > kmax ) {
                    vmax = v; 
                    kmax = kv;
                } // if
            } // if
        } // for

        newOrder.push_back( vmax );
        visited.at( vmax ) = true;
    } // for

    for ( int i : newOrder )
        cout << i << " ";

    vector<int> invertedList = findIndexI( newOrder );

    // Update edgeList with new indices
    for ( Edge & edge : edgeList ) {
        edge.src.id = invertedList.at(edge.src.id);
        edge.dst.id = invertedList.at(edge.dst.id);
    } // for

    return edgeList;
} // Gorder

// 把 CSR 寫入檔案
void writeCSRFile( string fileName, vector<int> OA, vector<int> EA ) {
    string name = fileName.substr( 0, fileName.find(".") );
    ofstream outputFile( name + "CSR" );

    outputFile << "AdjacencyGraph\n";
    outputFile << OA.size() << "\n";
    outputFile << EA.size() << "\n";

    for ( int val : OA )
        outputFile << val << "\n";

    for ( int val : EA )
        outputFile << val << "\n";

    outputFile.close();
} // writeCSRFile

// 把 reordering 後的 edgeList 寫入檔案
void writeEdgeListFile( string fileName, vector<Edge> edgeList, string oper ) {
    sortEdgeList( edgeList );
    ofstream outputFile(fileName.substr(0, fileName.find(".")) + oper + ".txt");

    for ( Edge edge : edgeList ) {
        outputFile << edge.src.id;
        outputFile << " ";
        outputFile << edge.dst.id;
        outputFile << " \n";
    } // for

    outputFile.close();
} // writeEdgeListFile

void recordReorder( vector<string> reorderNameList, vector<double> timeLogList ) {
    ofstream outputFile( "log.txt" );
    outputFile << "                Reorder " << "|" << "         Time\n";
    outputFile << "---------------------------------------\n";

    for ( int i = 0; i < timeLogList.size(); i++ ) {
        outputFile << setw(25) << reorderNameList.at(i) << setw(10) << timeLogList.at(i) << " ms\n";
        outputFile << "---------------------------------------\n";
    } // for

    outputFile.close();
} // recordReorder

void processCSR( string fileName ) {
    int numOfNodes = 0;
    vector<Edge> edgeList;
    vector<int> OA, EA;

    readEdgeList( fileName, edgeList, numOfNodes );
    el2CSR( edgeList, numOfNodes, OA, EA );
    writeCSRFile( fileName, OA, EA );
} // processCSR

void processReorder( string fileName ) {
    clock_t start, end;
    vector<double> timeLogList;
    vector<string> reorderNameList;

    int numOfNodes = 0;
    vector<Edge> edgeList, newOrder;
    readEdgeList( fileName, edgeList, numOfNodes );
    cout << "Read edgeList file finish.\n";

    start = clock();
    newOrder = Gorder( edgeList, numOfNodes );
    end = clock();
    timeLogList.push_back( (1000.0)*(double)(end-start)/CLOCKS_PER_SEC );
    reorderNameList.push_back( "G Order |" );
    writeEdgeListFile( fileName, newOrder, "_GO" );

    recordReorder( reorderNameList, timeLogList );
} // processReorder

int main( int argc, char *argv[] ) {
    if ( argc != 3 )
        cout << "----------Parameter Error----------\n";
    else {
        if ( strcmp( "init", argv[1] ) == 0 ) {
            string argv2(argv[2]);
            init( argv2 );
            cout << "init success!\n";
        } // if

        else if ( strcmp( "csr", argv[1] ) == 0 ) {
            string argv2(argv[2]);
            processCSR( argv2 );
        } // else if

        else if ( strcmp( "reorder", argv[1] ) == 0 ) {
            string argv2( argv[2] );
            processReorder( argv2 );
        } // else if

        else
            cout << "----------Error----------\n";

    } // else
} // main()
