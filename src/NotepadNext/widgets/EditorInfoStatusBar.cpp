/*
 * This file is part of Notepad Next.
 * Copyright 2022 Justin Dailey
 *
 * Notepad Next is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Notepad Next is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Notepad Next.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "EditorInfoStatusBar.h"
#include "MainWindow.h"
#include "StatusLabel.h"


EditorInfoStatusBar::EditorInfoStatusBar(QMainWindow *window) :
    QStatusBar(window)
{
    // Set up the status bar
    docType = new StatusLabel();
    addWidget(docType, 1);

    docSize = new StatusLabel(200);
    addPermanentWidget(docSize, 0);

    docPos = new StatusLabel(250);
    addPermanentWidget(docPos, 0);

    eolFormat = new StatusLabel(100);
    addPermanentWidget(eolFormat, 0);
    eolFormat->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(qobject_cast<StatusLabel*>(eolFormat), &StatusLabel::customContextMenuRequested, this, [=](const QPoint &pos) {
        emit customContextMenuRequestedForEOLLabel(eolFormat->mapToGlobal(pos));
    });

    unicodeType = new StatusLabel(125);
    addPermanentWidget(unicodeType, 0);

    overType = new StatusLabel(25);
    addPermanentWidget(overType, 0);

    MainWindow *w = qobject_cast<MainWindow *>(window);

    connect(w, &MainWindow::editorActivated, this, &EditorInfoStatusBar::connectToEditor);

    connect(qobject_cast<StatusLabel*>(overType), &StatusLabel::clicked, w, [=]() {
        ScintillaNext *editor = w->currentEditor();
        editor->editToggleOvertype();
        updateOverType(editor);
    });
}

void EditorInfoStatusBar::refresh(ScintillaNext *editor)
{
    updateDocumentSize(editor);
    updateSelectionInfo(editor);
    updateLanguage(editor);
    updateEol(editor);
    updateEncoding(editor);
    updateOverType(editor);
}

void EditorInfoStatusBar::connectToEditor(ScintillaNext *editor)
{
    // Remove any previous connections
    disconnect(editorUiUpdated);
    disconnect(documentLexerChanged);

    // Connect to the new editor
    editorUiUpdated = connect(editor, &ScintillaNext::updateUi, this, &EditorInfoStatusBar::editorUpdated);
    documentLexerChanged = connect(editor, &ScintillaNext::lexerChanged, this, [=]() { updateLanguage(editor); });

    refresh(editor);
}

void EditorInfoStatusBar::editorUpdated(Scintilla::Update updated)
{
    ScintillaNext *editor = qobject_cast<ScintillaNext *>(sender());

    if (Scintilla::FlagSet(updated, Scintilla::Update::Content)) {
        updateDocumentSize(editor);
    }

    if (Scintilla::FlagSet(updated, Scintilla::Update::Content) || Scintilla::FlagSet(updated, Scintilla::Update::Selection)) {
        updateSelectionInfo(editor);
    }
}

void EditorInfoStatusBar::updateDocumentSize(ScintillaNext *editor)
{
    QString sizeText = tr("Length: %L1    Lines: %L2").arg(editor->length()).arg(editor->lineCount());
    docSize->setText(sizeText);
}

void EditorInfoStatusBar::updateSelectionInfo(ScintillaNext *editor)
{
    QString selectionText;

    if (editor->selections() > 1) {
        selectionText = tr("Sel: N/A");
    }
    else {
        int start = editor->selectionStart();
        int end = editor->selectionEnd();
        int lines = editor->lineFromPosition(end) - editor->lineFromPosition(start);

        if (end > start)
            lines++;

        selectionText = tr("Sel: %L1 | %L2").arg(editor->countCharacters(start, end)).arg(lines);
    }

    const int pos = editor->currentPos();
    QString positionText = tr("Ln: %L1    Col: %L2    ").arg(editor->lineFromPosition(pos) + 1).arg(editor->column(pos) + 1);
    docPos->setText(positionText + selectionText);
}

void EditorInfoStatusBar::updateLanguage(ScintillaNext *editor)
{
    docType->setText(editor->languageName);
}


void EditorInfoStatusBar::updateEol(ScintillaNext *editor)
{
    // No good way to keep these in sync with the Main Window menu items :(

    switch(editor->eOLMode()) {
    case SC_EOL_CR:
        eolFormat->setText(tr("Macintosh (CR)"));
        break;
    case SC_EOL_CRLF:
        eolFormat->setText(tr("Windows (CR LF)"));
        break;
    case SC_EOL_LF:
        eolFormat->setText(tr("Unix (LF)"));
        break;
    }
}

void EditorInfoStatusBar::updateEncoding(ScintillaNext *editor)
{
    switch(editor->codePage()) {
    case 0:
        unicodeType->setText(tr("ANSI"));
        break;
    case SC_CP_UTF8:
        unicodeType->setText(tr("UTF-8"));
        break;
    default:
        unicodeType->setText(QString::number(editor->codePage()));
        break;
    }
}

void EditorInfoStatusBar::updateOverType(ScintillaNext *editor)
{
    bool overtype = editor->overtype();
    if (overtype) {
        //: This is a short abbreviation to indicate characters will be replaced when typing
        overType->setText(tr("OVR"));
    }
    else {
        //: This is a short abbreviation to indicate characters will be inserted when typing
        overType->setText(tr("INS"));
    }
}
