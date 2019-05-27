#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QList>

QString extraecodigo(QString cadena)
{
    int ini = cadena.indexOf("#",1);
    int fin = cadena.indexOf("#",ini+1);
    QString valcad = cadena.mid(ini+1,fin-ini-1);
    return valcad;
}

float extraepx(QString cadena)
{
    int ini = cadena.indexOf("\"",1);
    int fin = cadena.indexOf(",",ini+1);
    QString valcad = cadena.mid(ini+1,fin-ini-1);

    return valcad.toFloat();
}

float extraepy(QString cadena)
{
    int ini = cadena.indexOf(",",1);
    int fin = cadena.indexOf("\"",ini+1);
    QString valcad = cadena.mid(ini+1,fin-ini-1);

    return valcad.toFloat();
}

int buscalineas(QString codL, QStringList vcodL)
{
     return vcodL.indexOf(codL);
}

QString cadenaFLujo(QString Benv,QString Brec,QString valor, QString valor2, QStringList vcod, QList<float> vposx,QList<float> vposy){

    int posBe= vcod.indexOf(Benv);   // posicion en la lista de codigos de la barra de envio
    int posBr= vcod.indexOf(Brec);   // posicion en la lista de codigos de la barra de Recepcion

    float x = vposx.at(posBr)-vposx.at(posBe);
    float y = vposy.at(posBr)-vposy.at(posBe);
    QString cadmod = "";
    float flujo = valor.toFloat();
    float capacidad = valor2.toFloat();
    float sobrecarga=0;

    if (flujo <0.0) { // cuando el valor es negativo se invierte
        x*=-1; y*=-1; flujo *= -1;
    }
    cadmod = QString::number(flujo);
    cadmod += "/";
    sobrecarga = round(flujo/capacidad*100);   cadmod+=QString::number(sobrecarga);  cadmod+="%";

    if (x>0) cadmod.append("-&gt; ");  //↑ ↓ → ←
    if (x<0) cadmod.append("&lt;- ");
    if (y>0) cadmod.append("v");
    if (y<0) cadmod.append("^");
    //qDebug() << cadmod;

    return cadmod;
}

/* programa para escribir sobre las etiquetas de codigos, los valores de flujo de potencia
   y direccion de flujos, basado en posicion de etiquetas de barra envio y final

leexml [origen.dia] -i [resultados.csv] -o [Salida.dia] -b [barras.csv]*/


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString ArchOrigen("");
    QString ArchResultados("..");
    QString ArchBarras("..");
    QString ArchSalida("output.dia");

    QStringList myOptions;
    myOptions << "-l" << "-o"<<"-b";

    if(argc > 1 ) {
        ArchOrigen = argv[1];
        if(argc > 3) {
            switch (myOptions.indexOf(argv[2])) {
               case 0:
                    ArchResultados = argv[3];
                    break;
               case 1:
                    ArchSalida = argv[3];
                    break;
               case 2:
                    ArchBarras = argv[3];
                 break;
               default:
                    qDebug() << "el formato es: leexml [ArchOrigen.dia] -l [lineas.csv] -b [barras.csv] -o [ArchivoSalida.dia] ";

                    return 0;
                    break;
            }

        }
        if(argc > 5 ) {
            switch (myOptions.indexOf(argv[4])) {
               case 0:
                    ArchResultados = argv[5];
                    break;
               case 1:
                    ArchSalida = argv[5];
                    break;
               case 2:
                    ArchBarras = argv[5];
                    break;
               default:
                    qDebug() << "el formato es: leexml [ArchOrigen.dia] -l [lineas.csv] -b [barras.csv] -o [ArchivoSalida.dia] ";
                    return 0;
                    break;
            }
        }
        if(argc > 7 ) {
            switch (myOptions.indexOf(argv[6])) {
               case 0:
                    ArchResultados = argv[7];
                    break;
               case 1:
                    ArchSalida = argv[7];
                    break;
               case 2:
                    ArchBarras = argv[7];
                    break;
               default:
                    qDebug() << "el formato es: leexml [ArchOrigen.dia] -l [lineas.csv] -b [barras.csv] -o [ArchivoSalida.dia] ";
                    return 0;
                    break;
            }
        }


    }
    else {
        qDebug() << "el formato es: leexml [ArchOrigen.dia] -l [lineas.csv] -b [barras.csv] -o [ArchivoSalida.dia] ";
        qDebug() << "Nota: el formato del Lineas.csv tiene campos: Codigo/Benvio/Brecepcion/Flujo/Capacidad";
        qDebug() << "      el formato del barras.csv tiene campos: Codigo/Value";
        return 0;
    }

    qDebug() << "Archivos usados:";
    qDebug() << ArchOrigen;
    qDebug() << ArchResultados;
    qDebug() << ArchBarras;
    qDebug() << ArchSalida;

    QFile file(ArchOrigen);         // archivo origen
    QStringList archivomemoria;     // el archivo original y modificado en memoria (n valores)
    QStringList codigos;            // los codigos en memoria (k < n valores)
    QList<int> ucodigos;            // ubicacion de cada codifo en archivo original
    QStringList cordenadas;         // lista de coordenadas de cada etiqueta (k valores)
    QList<int> ucordenadas;         // ubicacion de codigo de coordenas
    QList<float> posx;              // posicion de cada coordenada x (k valores)
    QList<float> posy;              // Posicion de cada coordenada y (k valores)

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        qDebug() << "No existe archivo plantilla";
        return 0;
       }

// recorre el archivo origen, almacenando cada linea en "archivomemoria" y busca las etiquetas de acuerdo a cadena1,
// una vez ubicado, busca los codigos y la posicion x,y del codigo y lo almacena en codigos, ucodigos, coordenadas, ucordenadas, posx y posy


    QTextStream in(&file);
    bool flag=false;
    int  cflag =0;
    const QString cadena1 = "<dia:object type=\"Standard - Text\"";


       while (!in.atEnd()) {
             QString line = in.readLine();
             archivomemoria << line;

             if ( !flag && line.contains(cadena1, Qt::CaseInsensitive)) { // encuentra los textos standard
                  flag = true; cflag = 0;
             }

             if (flag && cflag < 4 &&  line.contains("<dia:point", Qt::CaseInsensitive)){ // ubica la posicion del texto
                 ucordenadas << archivomemoria.length();
                 cordenadas << line;
                 posx   << extraepx(line);
                 posy   << extraepy(line);
                 flag =true;
             }
             if ( flag && cflag < 12 && line.contains("<dia:string>", Qt::CaseInsensitive)) { // encuentra el codigo a reemplazar
                 ucodigos << archivomemoria.length();
                 codigos << extraecodigo(line);
                 flag = false; cflag = 0;
             }

             if (flag  && cflag >= 12) {  // cuando no encuentra la posicion
                 ucordenadas << 0;
                 cordenadas << "Error estructura";
                 flag =false;
             }

             cflag++;
             //i++;
             // qDebug() << line;
         }
       file.close();

// escribe archivo de codigos, ubicaciones y posicion xy encontrados

       qDebug() << "Leido :" << archivomemoria.length() << " Lineas, " << codigos.length() <<" codigos " << cordenadas.length() << "coordenadas";
       qDebug() << "Escribiendo archivo de codigos encontrados (codigos.csv)...";
       QFile data("codigos.csv");
       if (!data.open(QFile::WriteOnly | QFile::Truncate))
       {     qDebug() << "No se pudo crear archivo";
             return 0;
         }
       QTextStream out(&data);
       out << "Leido :" << archivomemoria.length() << " Lineas, " << codigos.length() <<" codigos " << cordenadas.length() << "coordenadas\n";
       out << "CODIGO, POSX, POSY, UBICA\n";

      int i=0;
      while ( i < codigos.length())
           out << codigos.at(i++)  << ","<< posx.at(i) << "," << posy.at(i) << "," << ucodigos.at(i) << "\n";
       data.close();

// modifica los codigos x valores encontrados desde el archivo de lineas

       QFile lineas(ArchResultados);
       QString cadFlujo;
       QString cadTemp;
       int ubiarchme;


       if (lineas.open(QIODevice::ReadOnly | QIODevice::Text))
       {
           qDebug() << "Inyectando valores de lineas en archivo grafico....";
           QTextStream inLineas(&lineas);
           QString line2 = inLineas.readLine(); //lee cabeceras
           while (!inLineas.atEnd()) {
               line2 = inLineas.readLine();
               QStringList dataL = line2.split(",");   //separacion del CSV en vector de datos
               ubiarchme = codigos.indexOf(dataL.at(0)); //ubicacion en vector codigos, con el numero se determina la linea
               cadTemp= archivomemoria.at(ucodigos.at(ubiarchme)-1);
               //qDebug() << "antes:" << archivomemoria.at(ucodigos.at(ubiarchme)-1);   // menos 1 ??
               cadFlujo = cadenaFLujo(dataL.at(1),dataL.at(2),dataL.at(3),dataL.at(4), codigos, posx,posy);
               //cambio de cadena temp incluyendo cadFlujo en lugar del codigo.
               cadTemp.replace(dataL.at(0),cadFlujo);
               archivomemoria.replace(ucodigos.at(ubiarchme)-1,cadTemp);
               //qDebug() << "despues:" <<  archivomemoria.at(ucodigos.at(ubiarchme)-1);   // menos 1 ??
               //qDebug() << dataL.at(0) << "=>>" << cadFlujo;
           }
        }
       else qDebug() << "Sin archivo de Lineas para Inyectar";
       lineas.close();

 // modifica los codigos x valores encontrados desde el archivo de Barras

      QFile barras(ArchBarras);
      cadFlujo="";
      cadTemp="";
      ubiarchme=0;


      if (barras.open(QIODevice::ReadOnly | QIODevice::Text))
      {
          qDebug() << "Inyectando valores de barras en archivo grafico....";
          QTextStream inLineas(&barras);
          QString line3 = inLineas.readLine(); //lee cabeceras
          while (!inLineas.atEnd()) {
              line3 = inLineas.readLine();
              QStringList dataL = line3.split(",");   //separacion del CSV en vector de datos 0,1
              ubiarchme = codigos.indexOf(dataL.at(0)); //ubicacion en vector codigos, con el numero se determina la linea
              cadTemp= archivomemoria.at(ucodigos.at(ubiarchme)-1);
              //qDebug() << "antes:" << archivomemoria.at(ucodigos.at(ubiarchme)-1);   // menos 1 ??
              cadFlujo = dataL.at(1);
              //cambio de cadena temp incluyendo cadFlujo en lugar del codigo.
              cadTemp.replace(dataL.at(0),cadFlujo);
              archivomemoria.replace(ucodigos.at(ubiarchme)-1,cadTemp);
              //qDebug() << "despues:" <<  archivomemoria.at(ucodigos.at(ubiarchme)-1);   // menos 1 ??
              //qDebug() << dataL.at(0) << "=>>" << cadFlujo;
          }
       }
      else qDebug() << "Sin archivo de Barras para Inyectar";
      barras.close();


   // escribe el archivo modificado
       qDebug() << "Escribiendo archivo grafico de resultado ("<< ArchSalida <<") ...";
       QFile salida(ArchSalida);
       if (!salida.open(QFile::WriteOnly | QFile::Truncate))
       {     qDebug() << "No se pudo crear archivo";
             return 0;
         }
       QTextStream modificado(&salida);
       i=0;
       while (i < archivomemoria.length())
           modificado << archivomemoria.at(i++) <<"\n";
       salida.close();


    return 0;

}

