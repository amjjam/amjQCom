isEmpty(prefix){
    prefix=/opt/amj
}

isEmpty(prefix_amjCom){
    prefix_amjCom=$${prefix}
}
isEmpty(INC){
  INC=/opt/amj/include /opt/QCustomPlot/include /opt/amj/include/amjCom
}
isEmpty(LIB){
  LIB=-L/opt/amj/lib -L/opt/QCustomPlot/lib
}

QT += widgets

TEMPLATE = lib
DEFINES += AMJQCOM_LIBRARY

CONFIG += c++11 debug

LIBS += $${LIB} -lamjCom

INCLUDEPATH += $${INC}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    amjQCom.C

HEADERS += \
    amjQCom_global.H \
    amjQCom.H

RESOURCES += resources.qrc

# Default rules for deployment.
target.path=$${prefix}/lib
headers.path=$${prefix}/include
headers.files=amjQCom.H

INSTALLS+= target headers


