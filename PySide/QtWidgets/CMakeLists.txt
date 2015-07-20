project(QtWidgets)


set(QtWidgets_OPTIONAL_SRC )
set(QtWidgets_DROPPED_ENTRIES )
## gone check_qt_class(Qt5Widgets QAbstractPageSetupDialog   QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QAbstractPrintDialog       QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QGtkStyle                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPageSetupDialog           QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintDialog               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintEngine               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintPreviewDialog        QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintPreviewWidget        QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrinter                   QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrinterInfo               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSessionManager            QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSizeGrip                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSystemTrayIcon            QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QMacStyle                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)

set(QtWidgets_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractbutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractgraphicsshapeitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractitemdelegate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractitemview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractscrollarea_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractslider_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qabstractspinbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qaction_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qactiongroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qapplication_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qboxlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qbuttongroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcalendarwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcheckbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcolordialog_wrapper.cpp
### Symbol not found: __ZN11QColumnView16staticMetaObjectE${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcolumnview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcombobox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcommandlinkbutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcommonstyle_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qcompleter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdatawidgetmapper_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdateedit_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdatetimeedit_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdesktopwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdial_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdialogbuttonbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdirmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdockwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qdoublespinbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qerrormessage_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qfiledialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qfileiconprovider_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qfilesystemmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qfocusframe_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qfontcombobox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qfontdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qformlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qframe_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgesture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgestureevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgesturerecognizer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsanchor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsanchorlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsblureffect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicscolorizeeffect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsdropshadoweffect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicseffect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsellipseitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsgridlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsitemanimation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsitemgroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicslayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicslayoutitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicslinearlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicslineitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsopacityeffect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicspathitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicspixmapitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicspolygonitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsproxywidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsrectitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsrotation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscale_wrapper.cpp
### no member named 'children' in 'QGraphicsItemGroup' ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscene_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenecontextmenuevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenedragdropevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicssceneevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenehelpevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenehoverevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenemouseevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenemoveevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicssceneresizeevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsscenewheelevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicssimpletextitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicstextitem_wrapper.cpp
### allocating an object of abstract class type '::QGraphicsTransformWrapper' ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicstransform_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicsview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgraphicswidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgridlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qgroupbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qhboxlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qheaderview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qinputdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qitemdelegate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qitemeditorcreatorbase_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qitemeditorfactory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qkeyeventtransition_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlabel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlayoutitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlcdnumber_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlineedit_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlistview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlistwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qlistwidgetitem_wrapper.cpp
### Symbol not found: __ZN11QMainWindow16staticMetaObjectE${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmainwindow_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmdiarea_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmdisubwindow_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmenu_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmenubar_wrapper.cpp
### Symbol not found: __ZN11QMessageBox16staticMetaObjectE${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmessagebox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qmouseeventtransition_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qpangesture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qpinchgesture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qplaintextdocumentlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qplaintextedit_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qprogressbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qprogressdialog_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qpushbutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qradiobutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qrubberband_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qscrollarea_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qscrollbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qshortcut_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qsizegrip_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qsizepolicy_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qslider_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qspaceritem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qspinbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qsplashscreen_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qsplitter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qsplitterhandle_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstackedlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstackedwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstatusbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyle_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleditemdelegate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstylefactory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstylehintreturn_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstylehintreturnmask_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstylehintreturnvariant_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoption_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionbutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptioncombobox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptioncomplex_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiondockwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionfocusrect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionframe_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiongraphicsitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiongroupbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionheader_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionmenuitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionprogressbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionrubberband_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionsizegrip_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionslider_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionspinbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontab_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontabbarbase_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontabwidgetframe_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontitlebar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontoolbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontoolbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptiontoolbutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstyleoptionviewitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qstylepainter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qswipegesture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qsystemtrayicon_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtabbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtableview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtablewidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtablewidgetitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtablewidgetselectionrange_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtabwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtapandholdgesture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtapgesture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtextbrowser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtextedit_extraselection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtextedit_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtilerules_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtimeedit_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtoolbar_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtoolbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtoolbutton_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtooltip_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtreeview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtreewidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtreewidgetitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtreewidgetitemiterator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qundocommand_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qundogroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qundostack_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qundoview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qvboxlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qwhatsthis_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qwidgetaction_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qwidgetitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qwizard_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qwizardpage_wrapper.cpp

${SPECIFIC_OS_FILES}
# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtwidgets_module_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/typesystem_widgets.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/typesystem_widgets.xml" @ONLY)

set(prev_shiboken_binary ${SHIBOKEN_BINARY})
string(REPLACE ";" " " QtWidgets_SRC_escape "${QtWidgets_SRC}")
file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh"
     "
echo \"$*\"
${prev_shiboken_binary} $*
python3 ${CMAKE_CURRENT_BINARY_DIR}/filter_init.py ${QtWidgets_SRC_escape}
     ")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/filter_init.py"
     "#! env python
import sys, os, re, pprint
pprint.pprint(sys.argv)
nset = set()
for fname in sys.argv[1:]:
    name = os.path.splitext(os.path.basename(fname))[0]
    print(name)
    if name.endswith('module_wrapper'):
        fn = fname
    else:
        name = name.split('_wrapper')
        assert name[1] == ''
        nset.add(name[0])
print(fn)
with open(fn) as f:
    lines = f.readlines()
removals = set()
for idx, line in enumerate(lines):
    res = re.search(' init_(\\w+)', line)
    if res and res.group(1).lower() not in nset:
        removals.add(res.group(1))
        lines[idx] = '//' + line
with open(fn, 'w') as f:
    f.writelines(lines)
removals = sorted(list(removals))
print('Removals:', removals)
")
file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

set(SHIBOKEN_BINARY "${CMAKE_CURRENT_BINARY_DIR}/fake_shiboken.sh")

# fake these variables for qt5
set(QtWidgets_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtWidgets_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets)

set(QtWidgets_typesystem_path "${QtCore_SOURCE_DIR}${PATH_SEP}${QtCore_BINARY_DIR}${PATH_SEP}${QtGui_SOURCE_DIR}${PATH_SEP}${QtGui_BINARY_DIR}${PATH_SEP}${QtWidgets_SOURCE_DIR}")

set(QtWidgets_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/
                        ${pyside_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        )
set(QtWidgets_libraries   pyside
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      )
set(QtWidgets_deps "QtGui")

create_pyside_module(QtWidgets
                     QtWidgets_include_dirs
                     QtWidgets_libraries
                     QtWidgets_deps
                     QtWidgets_typesystem_path
                     QtWidgets_SRC
                     ""
                     ${CMAKE_CURRENT_BINARY_DIR}/typesystem_widgets.xml)
